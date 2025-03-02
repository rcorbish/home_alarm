
#pragma once 

#include <iostream>

#include "decoder.hpp"

typedef void (*BroadcastFunction)( const char *, const int ) ;

constexpr const int SampleFrequency = 1000000 ;     // sample at 1MHz
constexpr const int CarrierFrequency = 344940000;//345000000 ;  // 344940000 ; // 345MHz is the carrier frequency
constexpr const int MaxPacketLength = 20000 ;       // 20mS = max packet length
constexpr const int EmptySignalLength = 1000 ;      // 1mS = no high signal means end of packet

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
        uint16_t relevantMeanThreshold ;
        uint16_t maxLow ;
        uint16_t minHigh ;

        void reset() ;
        void convertRawDataToSignal( unsigned char *buf, uint32_t len, uint16_t *cleanedSignal ) ;
        void processSignal( const uint16_t *buf, const uint32_t len ) ;
        virtual void publishPacket(const uint32_t startIndex, const uint16_t *cleanedSignal, const uint32_t numSamples) = 0 ;
        uint16_t median5FromArray(const uint16_t *arr ) ;

    public :
        SignalProcessor( const int _sampleFrequency ) ;
        void processRawBytes( unsigned char *buf, uint32_t len ) ;
} ;
