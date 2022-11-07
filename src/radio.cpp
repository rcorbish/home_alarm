
#include <math.h>
#include <libusb-1.0/libusb.h>
#include <cstring>
#include <ctime>
#include <iomanip>

#include "radio.hpp"

using namespace std ;

#define AMP_TO_DB(x) (10.0f * ((x) > 0 ? log10f(x) : 0) - 42.1442f)  // 10*log10f(16384.0f)
#define MAG_TO_DB(x) (20.0f * ((x) > 0 ? log10f(x) : 0) - 84.2884f)  // 20*log10f(16384.0f)


constexpr int SampleFrequency = 1000000 ;
constexpr int CarrierFrequency = 345000000; //344940000 ;

Radio::Radio() : threadId(0), rtlsdr_dev(0), dsp( SampleFrequency ) {
    device_count = rtlsdr_get_device_count();
    if (!device_count) {
        throw "No supported devices found" ;
    }

    int r = rtlsdr_open( &rtlsdr_dev, 0 ) ;
    
    r += rtlsdr_reset_buffer( rtlsdr_dev ) ;
    r += rtlsdr_set_center_freq( rtlsdr_dev, CarrierFrequency ) ;
    r += rtlsdr_set_tuner_gain_mode( rtlsdr_dev, 0 ) ;  // 0 = auto gain
    r += rtlsdr_set_sample_rate( rtlsdr_dev, SampleFrequency ) ; 
    // int numGains = rtlsdr_get_tuner_gains( rtlsdr_dev, nullptr ) ; 
    // int gains[numGains] ;
    // r += rtlsdr_get_tuner_gains( rtlsdr_dev, gains ) ; 
    // r += rtlsdr_set_tuner_gain( rtlsdr_dev, gains[0] ) ; // max gain 
    if( r ) {
        throw "Failed to init SDR device" ;
    }
}


Radio::~Radio() {
    if( threadId ) {
        rtlsdr_cancel_async( rtlsdr_dev ) ;
        pthread_join( threadId, nullptr ) ;
        threadId = 0 ;
    }
    if( rtlsdr_dev ) {
        rtlsdr_close( rtlsdr_dev ) ;
    }
}

/**
 * @brief Start the radio listening process in a new thread
 * 
 * @return int 
 */
int Radio::start() {
    pthread_create( &threadId, nullptr, Radio::startListening, this ) ;
    return 0 ;
}


/**
 * @brief Thread that begins receiving IQ data
 * 
 * @param self the instance that calls this thread
 * @return void* the return code from the thread - always nullptr
 */
void *Radio::startListening( void *self ) {
    Radio *radio = (Radio *)self ;
    radio->listen() ;   // blocking call
    return nullptr ;
}

/**
 * @brief This starts the callback process. It blocks on the async
 * read call. If it does return then something went wrong
 * 
 */
void Radio::listen() {
    int rc = rtlsdr_read_async( rtlsdr_dev, Radio::data_ready, this, 0, 0 ) ;
    if( rc<0 ) {
        std::cerr << "Check your RTL-SDR dongle, USB cables, and power supply" 
                    << libusb_error_name(rc) << "\n" 
                    << libusb_strerror(rc)  << std::endl ;
    }
}

/**
 * @brief Callback when data is received from the radio
 * 
 * @param buf 
 * @param len 
 * @param self 
 */
void Radio::data_ready( unsigned char *buf, uint32_t len, void *self ) {
    Radio *radio = (Radio *)self ;
    radio->dsp.processRawBytes( buf, len ) ;
}


std::ostream & operator << ( std::ostream &s, const Radio &radio ) {
    char vendor[256] ; 
    char product[256] ;
    char serial[256] ;

    rtlsdr_get_device_usb_strings( 0, vendor, product, serial ) ;
    s << "  Tuner:       " << rtlsdr_get_tuner_type( radio.rtlsdr_dev ) << "\n" 
      << "  Vendor:      " << vendor << "\n"
      << "  Product:     " << product << "\n"
      << "  Serial:      " << serial << "\n"
      << "  Frequency:   " << rtlsdr_get_center_freq( radio.rtlsdr_dev ) << "\n"
      << "  Freq. corr:  " << rtlsdr_get_freq_correction( radio.rtlsdr_dev ) << "\n"
      << "  Gain:        " << ( rtlsdr_get_tuner_gain( radio.rtlsdr_dev ) / 10.f ) << "\n"
      << "  Sample Rate: " << rtlsdr_get_sample_rate( radio.rtlsdr_dev ) 
      ;
    return s ;
}





