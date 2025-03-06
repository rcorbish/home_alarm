
#pragma once 

#include <cstdint>
#include <iostream>

typedef struct SensorEvent {
    std::uint32_t device_id ;

    bool contact     ;
    bool tamper      ;
    bool reed        ;
    bool alarm       ;
    bool battery_low ;
    bool heartbeat   ;

} SensorEvent ;

std::ostream & operator << ( std::ostream &s, const SensorEvent &ev ) ;
