

#include <iomanip>
#include "sensor_event.hpp"

std::ostream & operator << ( std::ostream &s, const SensorEvent &ev ) {
    s 
    << std::hex
    << " Device " << ev.device_id
    << std::dec
    << " Channel " << (int16_t)ev.channel
    << "\tReed " << ev.reed
    << " Contact " << ev.contact
    << " Hbeat "     << ev.heartbeat
    << " Lo-Batt " << ev.battery_low
    << " Tamper " << ev.tamper
    ;
    return s ;
}
