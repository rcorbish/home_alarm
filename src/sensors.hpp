
#pragma once

#include <string>
#include <map>

#include "sensor_event.hpp"

typedef struct SensorState {
    const std::string device_name ;
    bool active ;
    bool low_battery ;
    bool tamper ;
    time_t lastAlarm ;
    time_t lastEvent ;
    const char device_type ;
    SensorState( const std::string &name, const char type ) : 
                        device_name(name), 
                        lastEvent(0), 
                        lastAlarm(0), 
                        device_type(type) {} ; 
} SensorState ;

std::ostream & operator << ( std::ostream &s, const SensorState &state ) ;

class Sensors {
    private:
        std::map<uint32_t, SensorState> sensors;
    public:
        Sensors();
        void accept(const SensorEvent &event);
        std::string toString() const ;
};
