
#include <math.h>
#include <libusb-1.0/libusb.h>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <fstream>
#include "sensor_signal_processor.hpp"

using namespace std;

// void SensorSignalProcessor::processSignal(const uint16_t *cleanedSignal, const uint32_t numSamples)
// {
//     uint64_t quietSum = 0;
//     for (unsigned long i = 1; i < numSamples; i++) {
//         quietSum += cleanedSignal[i];
//     }

//     // LogData<uint16_t>( cleanedSignal, numSamples ) ;

//     uint16_t gapQuiet = (uint16_t)(((float)quietSum) / numSamples);

//     maxLow = max(500, (relevantMeanThreshold >> 1)); // 750 ;     // max value in low signal = noise
//     minHigh = relevantMeanThreshold << 1;            // 2000 ;    // min value in hi signal
//     // cout << "GAP: " << gapQuiet  << " " << noiseBackground << "   " << maxLow << "  -  " << minHigh << "\n" ;
//     // LogData<unsigned char>( buf, len ) ;

//     // uint64_t hiSum = 0 ;
//     // uint64_t loSum = 0 ;
//     // uint64_t hiNum = 0 ;
//     // uint64_t loNum = 0 ;
//     bool valid = false;

//     for (int i = 0; i < numSamples; i++) {
//         bool bit = cleanedSignal[i] > (previousBit ? maxLow : minHigh);
//         switch (dspState) {
//         case HIGH:
//             if (bit) {
//                 bitLength++;
//                 spikeLength = 0;
//                 // hiNum++ ;
//                 // hiSum += cleanedSignal[i] ;
//             } else {
//                 spikeLength++;
//                 if (spikeLength > MinRealPulseLength) {
//                     bits[bitIndex++] = '-';
//                     if (bitLength > longPulseLength)
//                         bits[bitIndex++] = '-';

//                     if (bitIndex > 1020) { // way too many bits w/o pause
//                         // cerr << "Confused signal - too many bits\n" ;
//                         reset();
//                     }

//                     // cout << bitLength << " H\n" ;

//                     bitLength = spikeLength;
//                     spikeLength = 0;
//                     dspState = LOW;
//                 }
//             }
//             break;
//         case IDLE:
//             // if( !bit ) {
//             //     maxLow = ( 7 * maxLow + cleanedSignal[i] ) >> 3 ;
//             //     minHigh = maxLow << 1 ;
//             // }
//         case LOW:
//             if (!bit) {
//                 bitLength++;
//                 spikeLength = 0;
//                 // loNum++ ;
//                 // loSum += cleanedSignal[i] ;

//                 if (bitIndex > 0 && bitLength > gapLength) {
//                     valid = decoder.parse(bits, bitIndex);
//                     reset();
//                 }
//             } else {
//                 spikeLength++;
//                 if (spikeLength > MinRealPulseLength) {
//                     bits[bitIndex++] = '_';
//                     if (dspState == LOW && bitLength > longPulseLength)
//                         bits[bitIndex++] = '_';

//                     if (bitIndex > 1020) { // way too many bits w/o pause
//                         // cerr << "Confused signal - too many bits\n" ;
//                         reset();
//                     }

//                     // cout << bitLength << " L\n" ;

//                     bitLength = spikeLength;
//                     spikeLength = 0;
//                     dspState = HIGH;
//                 }
//             }
//             break;
//         default:
//             reset();
//             break;
//         }
//     }
//     if (dspState == IDLE) {
//         relevantMeanThreshold = (3 * relevantMeanThreshold + gapQuiet) >> 2;
//     }
//     // if( valid ) {
//     //     cout << "HI: " << ((float) hiSum ) / hiNum  << "  LO: " << ((float) loSum ) / loNum  << "\n" ;
//     // }
// }


void SensorSignalProcessor::publishPacket(const uint32_t startIndex, const uint16_t *cleanedSignal, const uint32_t numSamples) {
    auto endIndex = min( numSamples, startIndex+MaxPacketLength ) ;
    auto *p = cleanedSignal + startIndex ;
    uint32_t sum = 0 ;
    char bits[1024] ;
    int numBits = 0 ;
    auto previousBit = false ;

    for( int i=startIndex ; i<(endIndex-5) ; i++, p++ ) {
        auto median = median5FromArray( p ) ;
        sum += median ;
    }
    auto mean = (sum / (endIndex-startIndex-5)) ;


    if( mean > relevantMeanThreshold ) {
        decoder.parse( bits, numBits );
    }
}
