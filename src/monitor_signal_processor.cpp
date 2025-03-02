
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
    if( endIndex - startIndex < 16000 ) return ;    // Need at least this many samples for a full packet

    stringstream ss;
    ss << "{ \"signal\": [ 0";
    // ss << cleanedSignal[startIndex-1] ;
    auto *p = cleanedSignal + startIndex ;
    uint32_t sum = 0 ;


    for( int i=0 ; i<2200 ; i++, p++ ) {
        auto median = median5FromArray( p ) ;
        sum += median ;
        // ss << ',' << median ;
    }
    auto mean = sum / 2200 ;

    p = cleanedSignal + startIndex ;
    auto y = *p++ ;
    // Implement low pass filter
    for( int i=startIndex+1 ; i<(endIndex-5) ; i++, p++) {
        y += ( *p - y ) / 8 ;   // This number can be adjusted based on your receiver & signals
        ss << ',' << (uint16_t)y ; //median ;
    }
    if( mean > relevantMeanThreshold ) {
        ss << "], \"mean\":" << mean << "}" ;
        broadcast(ss.str().c_str(), ss.str().length());
    }
}
