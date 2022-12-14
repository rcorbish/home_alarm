
#include <math.h>
#include <libusb-1.0/libusb.h>
#include <cstring>
#include <iomanip>

#include "decoder.hpp"

using namespace std ;


bool Decoder::parse( char *bits, int len ) {
    if( len < 32 ) 
        return false ; 

    // This saves doing lots of extraction - 1st 2 bytes = 0xFF 0xFE
    if( strncmp( "_-_-_-_-_-_-_-_-_-_-_-_-_-_-_--_", bits, 32 ) ) 
        return false ;

    // cerr << bits << " [" << len << "]\n";

    if( len == 127 ) {
        bits[len++] = '_' ;
    }
    if( len >= 128 ) {
        // if( memcmp( previousBits, bits, 128 ) == 0 ) 
        //     return true ; 

        uint8_t data[6] ;
        data[0] = 0 ;
        data[1] = 0 ;
        data[2] = 0 ;
        data[3] = 0 ;
        data[4] = 0 ;
        data[5] = 0 ;
        
        char *p = bits + 32 ;
        int byte = 0 ;
        for( int i=0 ; i<96 ; i+=2 ) {      // get 6 bytes of data to a buffer
            data[byte] <<= 1 ;
            if( p[i] == '_' ) data[byte] |= 1 ;
            if( i==14 || i==30 || i==46 || i==62 || i==78 ) byte++ ;
        }

        // cout << hex << (uint16_t)data[0] << ' ' << (uint16_t)data[1] << ' ' 
        //         << (uint16_t)data[2] << ' ' << (uint16_t)data[3] << ' ' 
        //         << (uint16_t)data[4] << ' ' << (uint16_t)data[5] << "\n" ;

        uint8_t channel    = data[0] >> 4;
        uint32_t device_id = ((data[0] & 0xf) << 16) | (data[1] << 8) | data[2];
        uint16_t crc       = (data[4] << 8) | data[5];

        uint16_t polynomial = 
            (channel == 0x2 || channel == 0x4 || channel == 0xA) ?
            0x8050 : 0x8005 ;
        uint16_t calculatedCrc = crc16( data, 4, polynomial ) ;
        if( calculatedCrc == crc ) {
            memcpy( previousBits, bits, 128 ) ;
            SensorEvent ev { 
                .channel     = channel ,
                .device_id   = device_id ,
                .contact     = (data[3] & 0x80) != 0 , 
                .tamper      = (data[3] & 0x40) != 0 , 
                .reed        = (data[3] & 0x20) != 0 , 
                .alarm       = (data[3] & 0x10) != 0 , 
                .battery_low = (data[3] & 0x08) != 0 , 
                .heartbeat   = (data[3] & 0x04) != 0 
            } ;
            sensorEventProcessor.receive( ev ) ;
            return true ;
        }
    // } else {
    //   cerr << bits << " [" << len << "]\n";
    }
    return false ;
} 

uint16_t Decoder::crc16( const uint8_t *message, int nBytes, uint16_t polynomial )
{
    uint16_t remainder = 0;

    for( int byte = 0; byte < nBytes; ++byte ) {
        remainder ^= message[byte] << 8;
        for( int bit = 0; bit < 8; ++bit ) {
            if (remainder & 0x8000) {
                remainder = (remainder << 1) ^ polynomial;
            }
            else {
                remainder = (remainder << 1);
            }
        }
    }
    return remainder;
}



