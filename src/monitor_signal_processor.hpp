
#pragma once 

#include <iostream>
#include "signal_processor.hpp"


class MonitoringSignalProcessor : public SignalProcessor {
    private:
        BroadcastFunction broadcast ;
    protected:
        virtual void processSignal( const uint16_t *cleanedSignal, const uint32_t numSamples ) ;
        void publishPacket(const uint32_t startIndex, const uint16_t *cleanedSignal, const uint32_t numSamples) ;

    public :
        MonitoringSignalProcessor( const int _sampleFrequency, BroadcastFunction _broadcast ) :
            SignalProcessor( _sampleFrequency ), broadcast(_broadcast) {} ;
} ;
