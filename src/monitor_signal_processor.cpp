
#include <math.h>
#include <libusb-1.0/libusb.h>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <fstream>
#include "monitor_signal_processor.hpp"

using namespace std;

void MonitoringSignalProcessor::processSignal(uint16_t *cleanedSignal, uint32_t numSamples)
{
    uint64_t quietSum = 0;
    for (unsigned long i = 1; i < numSamples; i++) {
        quietSum += cleanedSignal[i];
    }

    for (int i = 0; i < numSamples; i++) {
        bool bit = cleanedSignal[i] > (previousBit ? maxLow : minHigh);
        switch (dspState) {
        case HIGH:
            if (bit) {
                bitLength++;
                spikeLength = 0;
            } else {
                spikeLength++;
                if (spikeLength > MinRealPulseLength) {
                    bits[bitIndex++] = '-';
                    if (bitLength > longPulseLength)
                        bits[bitIndex++] = '-';

                    if (bitIndex > 1020) { // way too many bits w/o pause
                        reset();
                    }

                    bitLength = spikeLength;
                    spikeLength = 0;
                    dspState = LOW;
                }
            }
            break;
        case IDLE:
        case LOW:
            if (!bit) {
                bitLength++;
                spikeLength = 0;
            } else {
                spikeLength++;
                if (spikeLength > MinRealPulseLength) {
                    bits[bitIndex++] = '_';
                    if (dspState == LOW && bitLength > longPulseLength)
                        bits[bitIndex++] = '_';

                    if (bitIndex > 1020) { // way too many bits w/o pause
                        reset();
                    }

                    bitLength = spikeLength;
                    spikeLength = 0;
                    dspState = HIGH;
                }
            }
            break;
        default:
            reset();
            break;
        }
    }
}

