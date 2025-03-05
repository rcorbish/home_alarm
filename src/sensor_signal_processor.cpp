
#include <math.h>
#include <libusb-1.0/libusb.h>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <fstream>
#include "sensor_signal_processor.hpp"

using namespace std;


void SensorSignalProcessor::publishPacket(const uint32_t startIndex, const uint16_t *cleanedSignal, const uint32_t numSamples) {
    auto endIndex = min( numSamples, startIndex+MaxPacketLength ) ;
    auto *p = cleanedSignal + startIndex ;
    uint32_t sum = 0 ;
    char bits[1024] ;
    int bitIndex = 0 ; 
    int bitLength = 0 ;
    int numBits = 0 ;
    auto currentBit = false ;

    if( endIndex - startIndex < 16000 ) return ;    // Need at least this many samples for a full packet

    for( int i=0 ; i<2200 ; i++, p++ ) {
        auto median = median5FromArray( p ) ;
        sum += median ;
    }
    auto mean = sum / 2200 ;    // mean will be used to define high & low signals
    
    p = cleanedSignal + startIndex ;
    auto y = *p++ ;
    // Implement low pass filter
    for( int i=startIndex ; i<endIndex ; i++, p++) {
        y += ( *p - y ) / 8 ;   // This number can be adjusted based on your receiver & signals

        if( y > mean ) {    // in a high bit
            if( currentBit ) {  // still in a bit ? 
                bitLength++ ;   // then increment the length
            } else {
                bits[bitIndex++] = '_' ;
                if( bitLength> longPulseLength ) {
                    bits[bitIndex++] = '_' ;
                }
                bitLength = 0 ; // now we're in a new bit
                currentBit = true ;
            }
        } else {
            if( currentBit ) {
                bits[bitIndex++] = '-' ;
                if( bitLength> longPulseLength ) {
                    bits[bitIndex++] = '-' ;
                }
                bitLength = 0 ; // now we're in a new bit
                currentBit = false ;
            } else {
                bitLength++ ;   // then increment the length
            }
        }
    }

    if( mean > relevantMeanThreshold ) {
        decoder.parse( bits, bitIndex );
    }
}
