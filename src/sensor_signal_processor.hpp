
#pragma once 

#include <iostream>

#include "signal_processor.hpp"
#include "decoder.hpp"


class SensorSignalProcessor : public SignalProcessor {
    protected:
        Decoder decoder ;
        virtual void processSignal( const uint16_t *buf, const uint32_t len ) ;
    public :
        SensorSignalProcessor( const int _sampleFrequency) :
            SignalProcessor( _sampleFrequency ) {} ;
} ;

