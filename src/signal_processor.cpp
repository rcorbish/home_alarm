
#include <math.h>
#include <libusb-1.0/libusb.h>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <fstream>
#include "signal_processor.hpp"

using namespace std;

constexpr float MinLevel = 75; // Carrier noise level

template <class T>
void LogData(const T *data, const size_t len);

#define F_SCALE 15
#define S_CONST (1 << F_SCALE)
#define FIX(x) ((int)(x * S_CONST))

SignalProcessor::SignalProcessor(const int sampleFrequency) : longPulseLength(sampleFrequency / 6000), // uS definition of long pulse
                                                              gapLength(sampleFrequency / 1000)        // uS definition of inter envelope gap
{
    relevantMeanThreshold = 750;
    maxLow = 150;   // max value in low signal = noise
    minHigh = 700;  // min value in hi signal

    reset();
}



void SignalProcessor::convertRawDataToSignal(unsigned char *buf, uint32_t len, uint16_t *signal)
{
    // cout << "Process " << len << " raw bytes\n" ;
    const int numSamples = len >> 1;
    uint16_t rawSignal[numSamples];

    uint64_t sum = 0; // max is 0x40000 * 0xffff  ( unsigned )

    auto *p = buf;
    uint16_t *s = rawSignal;
    // 1st convert IQ to raw magnitude ( squared )
    for (int i = 0; i < numSamples; i++) {
        int16_t x = 127 - *p++;
        int16_t y = 127 - *p++;
        *s = x * x + y * y;
        sum += *s++;
    }

    float baseLine = (float)sum / len;
    if (baseLine < MinLevel) { // just noise in the received IQ data ?
        if (bitLength == 0) {
            return;
        }
        // decoder.parse( bits, bitIndex ) ;
        reset();
        return;
    }
    // cout << baseLine << "\n" ;
    // LogData<unsigned char>( buf, len ) ;

    static int const a[2] = {FIX(1.00000) >> 1, FIX(0.85408) >> 1};
    static int const b[2] = {FIX(0.07296) >> 1, FIX(0.07296) >> 1};

    signal[0] = (a[1] * state_y + b[0] * (rawSignal[0] + state_x)) >> (F_SCALE - 1);
    for (unsigned long i = 1; i < numSamples; i++) {
        signal[i] = (a[1] * rawSignal[i - 1] + b[0] * (rawSignal[i] + rawSignal[i - 1])) >> (F_SCALE - 1);
    }
    state_x = buf[numSamples - 1];
    state_y = rawSignal[numSamples - 1];
}

void SignalProcessor::processRawBytes(unsigned char *buf, uint32_t len)
{
    const int numSamples = len >> 1 ;

    uint16_t cleanedSignal[numSamples] ;
    convertRawDataToSignal(buf, len, cleanedSignal) ;
    processSignal(cleanedSignal, numSamples) ;
}

void SignalProcessor::reset()
{
    bitIndex = 0;
    bitLength = 0;
    spikeLength = 0;
    previousBit = false;
    dspState = IDLE;
}

void SignalProcessor::processSignal(const uint16_t *cleanedSignal, const uint32_t numSamples)
{
    LogData<uint16_t>(cleanedSignal, numSamples);

    uint32_t startIndex = 0 ;
    uint32_t endIndex = numSamples ;
    auto dspState = IDLE;
    int bitLength = 0;
    int wrongBitLength = 0;
    int bitIndex = 0;
    int packetLength = 0;
    int pulseLength = 0;

    int maxPulseLength = 0;

    uint16_t sum = 0 ;
    for (int i = 0; i < numSamples; i++) {
        sum ^= cleanedSignal[i] ;
    }
    cout << "XOR = " << sum << endl ;
    return ;

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
                    if( startIndex > 0 && bitLength > EmptySignalLength ) {
                        cout << "Packet len " << packetLength << " bit length " << bitLength << " start IX " << startIndex << endl;
                        if( packetLength >= MaxPacketLength && startIndex > 0 ) {
                            cout << "Publish packet\n" ;
//                            publishPacket( startIndex, cleanedSignal, numSamples ) ;
                        }
                        packetLength = 0 ;
                        startIndex = 0 ;
                        wrongBitLength = 0 ;
                        dspState = IDLE ;
                        bitLength = 0 ;
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

uint16_t SignalProcessor::median5FromArray(const uint16_t *arr ) {
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

template <class T>
void LogData(const T *data, const size_t len)
{
    int i ;
    for (i = 0; i < len; i++) {
        if( *data != 0 ) {
            break ;
        }
    }
    if( i == len ) {
        return ;
    }

    ofstream dataFile("signal.csv") ;
    for (; i < len; i++) {
        if ((i & 31) == 0)
            dataFile << "\n"
                     << setw(8) << hex << uppercase << i << ":  ";
        dataFile << hex << uppercase << (int)(*data++) << " ";
    }
    dataFile << "---------------------------------------------\n";
    dataFile.flush();
    dataFile.close();
}

template <uint16_t> void LogData(const uint16_t *data, const size_t len) ;
