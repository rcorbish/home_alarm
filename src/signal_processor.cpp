
#include <math.h>
#include <libusb-1.0/libusb.h>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <fstream>
#include "signal_processor.hpp"

using namespace std ;

constexpr float MinLevel = 150 ;   // Carrier noise level

template <class T> void LogData( T *data, size_t len ) ;

SignalProcessor::SignalProcessor( const int sampleFrequency ) : 
        longPulseLength( sampleFrequency / 6000 ), // uS definition of long pulse
        gapLength( sampleFrequency / 1000 ) // uS definition of inter envelope gap
{   
    noiseBackground = 1000 ;
    maxLow = 750 ;      // max value in low signal = noise
    minHigh = 2000 ;    // min value in hi signal

    reset() ;
} ;


#define F_SCALE 15
#define S_CONST (1 << F_SCALE)
#define FIX(x) ((int)(x * S_CONST))



void SignalProcessor::processRawBytes( unsigned char *buf, uint32_t len ) {

    // cout << "Process " << len << " raw bytes\n" ;
    const int numSamples = len >> 1 ;
    uint16_t signal[ numSamples ] ;

    uint64_t sum = 0;   // max is 0x40000 * 0xffff  ( unsigned )

    auto *p = buf ;
    uint16_t *s = signal ;
    // 1st convert IQ to raw magnitude ( squared )
    for( int i=0 ; i<numSamples ; i++ ) {
        int16_t x = 127 - *p++ ;
        int16_t y = 127 - *p++ ;
        *s = x * x + y * y ; 
        sum += *s++ ;
    }

    float baseLine = (float)sum / len ;
    if( baseLine < MinLevel ) {   // just noise in the received IQ data ?
        if( bitLength == 0 ) {
            return ;
        }
        decoder.parse( bits, bitIndex ) ;
        reset() ;
        return ;
    }
    // cout << baseLine << "\n" ;
    // LogData<unsigned char>( buf, len ) ;

    static int const a[2] = {FIX(1.00000) >> 1, FIX(0.85408) >> 1};
    static int const b[2] = {FIX(0.07296) >> 1, FIX(0.07296) >> 1};

    uint16_t cleanedSignal[numSamples] ;

    uint64_t quietSum = 0 ;

    cleanedSignal[0] = (a[1] * state_y + b[0] * (signal[0] + state_x)) >> (F_SCALE - 1); 
    for (unsigned long i = 1; i < numSamples ; i++) {
        cleanedSignal[i] = (a[1] * cleanedSignal[i - 1] + b[0] * (signal[i] + signal[i - 1])) >> (F_SCALE - 1); 
        quietSum += cleanedSignal[i] ;
    }

    state_x = buf[numSamples-1] ;
    state_y = cleanedSignal[numSamples-1] ;

    // LogData<uint16_t>( cleanedSignal, numSamples ) ;

    uint16_t gapQuiet = (uint16_t)( ((float) quietSum ) / numSamples ) ;

    maxLow = max( 500, (noiseBackground >> 1) ) ; // 750 ;     // max value in low signal = noise
    minHigh = noiseBackground << 1 ; // 2000 ;    // min value in hi signal
    // cout << "GAP: " << gapQuiet  << " " << noiseBackground << "   " << maxLow << "  -  " << minHigh << "\n" ;
    // LogData<unsigned char>( buf, len ) ;

    // uint64_t hiSum = 0 ;
    // uint64_t loSum = 0 ;
    // uint64_t hiNum = 0 ;
    // uint64_t loNum = 0 ;
    bool valid = false ;

    for( int i=0 ; i<numSamples ; i++ ) {
        bool bit =  cleanedSignal[i] > ( previousBit ? maxLow : minHigh ) ;
        switch ( dspState ) {
            case HIGH :
                if( bit ) {
                    bitLength++ ;
                    spikeLength = 0 ;                    
                    // hiNum++ ;
                    // hiSum += cleanedSignal[i] ;
                } else {
                    spikeLength++ ;
                    if( spikeLength>MinRealPulseLength ) {                    
                        bits[bitIndex++] = '-' ;
                        if( bitLength>longPulseLength ) bits[bitIndex++] = '-' ;

                        if( bitIndex > 1020 ) { // way too many bits w/o pause
                            // cerr << "Confused signal - too many bits\n" ;
                            reset() ;
                        }

                        // cout << bitLength << " H\n" ;

                        bitLength = spikeLength ;                   
                        spikeLength = 0 ; 
                        dspState = LOW ;
                    }
                }
                break ;
            case IDLE :
                // if( !bit ) {
                //     maxLow = ( 7 * maxLow + cleanedSignal[i] ) >> 3 ; 
                //     minHigh = maxLow << 1 ;
                // }
            case LOW :
                if( !bit ) {
                    bitLength++ ;
                    spikeLength = 0 ;
                    // loNum++ ;
                    // loSum += cleanedSignal[i] ;

                    if( bitIndex>0 && bitLength>gapLength ) {                        
                        valid = decoder.parse( bits, bitIndex ) ;
                        reset() ;
                    }
                } else {
                    spikeLength++ ;
                    if( spikeLength>MinRealPulseLength ) {                    
                        bits[bitIndex++] = '_' ;
                        if( dspState==LOW && bitLength>longPulseLength ) bits[bitIndex++] = '_' ;

                        if( bitIndex > 1020 ) { // way too many bits w/o pause
                            // cerr << "Confused signal - too many bits\n" ;
                            reset() ;
                        }

                        // cout << bitLength << " L\n" ;

                        bitLength = spikeLength ;
                        spikeLength = 0 ;
                        dspState = HIGH ;
                    }
                }
                break ;
            default:
                reset() ;
                break ;
        }
    }
    if( dspState==IDLE ) {
        noiseBackground = ( 3 * noiseBackground + gapQuiet ) >> 2 ;
    }
    // if( valid ) {
    //     cout << "HI: " << ((float) hiSum ) / hiNum  << "  LO: " << ((float) loSum ) / loNum  << "\n" ;
    // }
}


void SignalProcessor::reset() {
    bitIndex = 0 ;
    bitLength = 0 ;
    spikeLength = 0 ;
    previousBit = false ; 
    dspState = IDLE ;
}

template <class T>
void LogData( T *data, size_t len ) {
    ofstream dataFile( "signal.csv" ) ;
    for( int i=0 ; i<len ; i++ ) {
        if( (i & 31) == 0 ) dataFile << "\n"  << setw(8) <<  hex << uppercase << i << ":  " ;
        dataFile << hex << uppercase << (int)( *data++ ) << " " ;
    }
    dataFile << "---------------------------------------------\n" ;
    dataFile.flush() ;
    dataFile.close() ;
}
