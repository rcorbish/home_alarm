
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
void LogData(T *data, size_t len);

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

template <class T>
void LogData(T *data, size_t len)
{
    ofstream dataFile("signal.csv");
    for (int i = 0; i < len; i++) {
        if ((i & 31) == 0)
            dataFile << "\n"
                     << setw(8) << hex << uppercase << i << ":  ";
        dataFile << hex << uppercase << (int)(*data++) << " ";
    }
    dataFile << "---------------------------------------------\n";
    dataFile.flush();
    dataFile.close();
}
