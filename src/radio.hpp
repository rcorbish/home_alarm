
#pragma once 

#include <iostream>
#include <pthread.h>
#include <rtl-sdr.h>

#include "signal_processor.hpp"

class Radio {
    friend std::ostream & operator << ( std::ostream &s, const Radio &radio ) ;
    static void data_ready( unsigned char *buf, uint32_t len, void *self ) ;
    static void *startListening( void *self ) ;

    pthread_t threadId ;
    rtlsdr_dev_t *rtlsdr_dev ;
    uint32_t device_count ;

    SignalProcessor dsp ;

protected:
    void listen() ;

public:
    Radio() ;
    virtual ~Radio() ;
    int start() ;

} ;

std::ostream & operator << ( std::ostream &s, const Radio &radio ) ;

