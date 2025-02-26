
#pragma once 

#include <iostream>
#include "signal_processor.hpp"


class MonitoringSignalProcessor : public SignalProcessor {
    protected:
        virtual void processSignal( uint16_t *cleanedSignal, uint32_t numSamples ) ;
    public :
        MonitoringSignalProcessor( const int _sampleFrequency ) :
            SignalProcessor( _sampleFrequency ) {} ;
} ;
