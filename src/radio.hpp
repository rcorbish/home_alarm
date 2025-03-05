
#pragma once 

#include <pthread.h>
#include <rtl-sdr.h>

#include "signal_processor.hpp"

class Radio {
private:
    friend std::ostream & operator << ( std::ostream &s, const Radio &radio ) ;

    static void data_ready( unsigned char *buf, uint32_t len, void *self ) ;
    static void *startListening( void *self ) ;
    SignalProcessor *dsp ;

    pthread_t threadId ;
    rtlsdr_dev_t *rtlsdr_dev ;
    uint32_t device_count ;

    Radio( SignalProcessor *_dsp ) ;

protected:
    void listen() ;
public:
    static Radio *getSensorInstance() ;
    static Radio *getMonitoringInstance(BroadcastFunction broadcast) ;
    virtual ~Radio() ;
    int start() ;
};

std::ostream& operator<<(std::ostream& os, const Radio& radio) ;
