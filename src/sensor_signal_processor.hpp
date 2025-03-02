
#pragma once 

#include <iostream>

#include "signal_processor.hpp"
#include "decoder.hpp"


class SensorSignalProcessor : public SignalProcessor {
    protected:
        Decoder decoder ;
        void publishPacket(const uint32_t startIndex, const uint16_t *cleanedSignal, const uint32_t numSamples) ;
    public :
        SensorSignalProcessor( const int _sampleFrequency) :
            SignalProcessor( _sampleFrequency ) {} ;
} ;

