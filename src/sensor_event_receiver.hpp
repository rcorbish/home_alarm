
#pragma once

#include <string>
#include <map>

#include "sensor_event.hpp"

typedef struct SensorState {
    const std::string device_name ;
    bool active ;
    bool low_battery ;
    bool tamper ;
    time_t lastEvent ;

    SensorState( const std::string &name ) : device_name(name) {}
} SensorState ;

std::ostream & operator << ( std::ostream &s, const SensorState &state ) ;

class SensorEventProcessor {
    private:
        std::map<uint32_t,SensorState> sensors ;
    public:
        SensorEventProcessor() ;
        void receive( const SensorEvent &event ) ;
} ;
