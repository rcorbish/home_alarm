
#pragma once 

#include <iostream>

#include "sensor_event_receiver.hpp"

/**
 * @brief Decodes a bit-stream into the Security Device message
 * 
 *   PREAMBLE   = FF FE   16 bit constant
 *   CHANNEL    = C       4 bits
 *   DEVICE     = DDD     12 bits
 *   STATUS     = S       8 bits
 *   CRC        = XXXX    16 bits
 * 
 *  This maintains a state to optimize - previousBits. If this
 *  doesn't change, no need to repeat messages. Devices seem to 
 *  send multiple copies of the same message.
 */
class Decoder {
    private:
        char previousBits[128] ;
        SensorEventProcessor sensorEventProcessor ;

    protected:
        uint16_t crc16( const uint8_t *message, const int nBytes, uint16_t polynomial ) ;

    public:
        Decoder() {} ; 
        bool parse( char *bits, int len ) ;
} ;
