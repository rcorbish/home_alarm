
#include <math.h>
#include <libusb-1.0/libusb.h>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <fstream>
#include "monitor_signal_processor.hpp"

using namespace std;

uint16_t median5FromArray(const uint16_t *arr ) ;

void MonitoringSignalProcessor::processSignal(const uint16_t *cleanedSignal, const uint32_t numSamples)
{
    uint32_t startIndex = 0 ;
    uint32_t endIndex = numSamples ;
    auto dspState = IDLE;
    int bitLength = 0;
    int wrongBitLength = 0;
    int bitIndex = 0;
    int packetLength = 0;
    int pulseLength = 0;

    int maxPulseLength = 0;

    for (int i = 0; i < numSamples; i++) {
        packetLength++ ;
        switch (dspState) {
            case HIGH: {
                auto bit = cleanedSignal[i] > maxLow ;
                if (bit) {
                    wrongBitLength = 0;
                    if( startIndex == 0 ) {
                        startIndex = i - bitLength;
                    }
                    bitLength++;
                } else {
                    wrongBitLength++ ;
                    if (wrongBitLength > MinRealPulseLength) {
                        bitLength = wrongBitLength;
                        wrongBitLength = 0;
                        dspState = LOW;
                    }
                }
                break;
            }
            case IDLE:
            case LOW: {
                auto bit = cleanedSignal[i] > minHigh ;

                if (!bit) {
                    if( bitLength > EmptySignalLength ) {
                        if( packetLength >= MaxPacketLength && startIndex > 0 ) {
                            publishPacket( startIndex, cleanedSignal, numSamples ) ;
                        }
                        packetLength = 0 ;
                        startIndex = 0 ;
                        wrongBitLength = 0 ;
                        dspState = IDLE ;
                    }
                    bitLength++;
                    wrongBitLength = 0;
                } else {
                    wrongBitLength++ ;
                    if (wrongBitLength > MinRealPulseLength) {
                        bitLength = wrongBitLength;
                        wrongBitLength = 0;
                        dspState = HIGH;
                    }
                }
                break;
            }
            default:
                break;
        }
    }
}



void MonitoringSignalProcessor::publishPacket(const uint32_t startIndex, const uint16_t *cleanedSignal, const uint32_t numSamples) {
    auto endIndex = min( numSamples, startIndex+MaxPacketLength ) ;
    stringstream ss;
    ss << "{ \"signal\": [";
    ss << cleanedSignal[startIndex-1] ;
    auto *p = cleanedSignal + startIndex ;
    uint32_t sum = 0 ;
    for( int i=startIndex ; i<(endIndex-5) ; i++, p++ ) {
        auto median = median5FromArray( p ) ;
        sum += median ;
        ss << ',' << median ;
    }
    auto mean = (sum / (endIndex-startIndex-5)) ;
    if( mean > relevantMeanThreshold ) {
        ss << "], \"mean\":" << mean << "}" ;
        broadcast(ss.str().c_str(), ss.str().length());
    }
}

uint16_t median5FromArray(const uint16_t *arr ) {
    // We'll track min/median/max of first three elements
    uint16_t min_abc, med_abc, max_abc;
    
    // Find ordering of arr[0] and arr[1] (Comparison 1)
    if (arr[0] > arr[1]) {
        min_abc = arr[1];
        max_abc = arr[0];
    } else {
        min_abc = arr[0];
        max_abc = arr[1];
    }
    
    // Add arr[2] to the ordering (Comparisons 2-3)
    if (arr[2] > max_abc) {
        med_abc = max_abc;
        max_abc = arr[2];
    } else if (arr[2] < min_abc) {
        med_abc = min_abc;
        min_abc = arr[2];
    } else {
        med_abc = arr[2];
    }
    
    // Order arr[3] and arr[4] (Comparison 4)
    uint16_t min_de, max_de;
    if (arr[3] > arr[4]) {
        min_de = arr[4];
        max_de = arr[3];
    } else {
        min_de = arr[3];
        max_de = arr[4];
    }
    
    // Find the median of all 5 elements (Comparisons 5-6)
    if (med_abc > max_de) {
        // Median is either max_de or min_abc
        return std::max(min_abc, max_de);
    } else if (min_de > med_abc) {
        // Median is either min_de or max_abc
        return std::min(min_de, max_abc);
    } else {
        // med_abc is the median
        return med_abc;
    }
}