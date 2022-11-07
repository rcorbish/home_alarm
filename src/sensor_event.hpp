
#pragma once 

#include <iostream>

typedef struct SensorEvent {
    uint8_t channel    ;
    uint32_t device_id ;

    bool contact     ;
    bool tamper      ;
    bool reed        ;
    bool alarm       ;
    bool battery_low ;
    bool heartbeat   ;

} SensorEvent ;

std::ostream & operator << ( std::ostream &s, const SensorEvent &ev ) ;
