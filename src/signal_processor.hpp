
#pragma once 

#include <iostream>

#include "decoder.hpp"

constexpr const int SampleFrequency = 1000000 ;
constexpr const int CarrierFrequency = 345000000; //344940000 ;

typedef enum DSP_STATE {
    HIGH,
    LOW,
    IDLE
} DSP_STATE ;


/**
 * @brief This is a stateful decoder, to gather Manchester encoded
 * signal into a buffer of 1s and 0s. This is NOT thread safe, it has
 * to maintain state between batches of raw signal
 * 
 */
class SignalProcessor {
    protected:
        const int longPulseLength ;
        const int gapLength ;
        static constexpr int MinRealPulseLength = 10 ;

        int16_t state_x ;
        int16_t state_y ;
        char bits[1024] ;
        bool previousBit ; 
        int bitLength ;
        int spikeLength ;
        int bitIndex ;
        DSP_STATE dspState ;
        int eventStart ;
        uint16_t noiseBackground ;
        uint16_t maxLow ;
        uint16_t minHigh ;

        void reset() ;
        void convertRawDataToSignal( unsigned char *buf, uint32_t len, uint16_t *cleanedSignal ) ;
        virtual void processSignal( uint16_t *buf, uint32_t len ) = 0 ;
    public :
        SignalProcessor( const int _sampleFrequency ) ;
        void processRawBytes( unsigned char *buf, uint32_t len ) ;
} ;
