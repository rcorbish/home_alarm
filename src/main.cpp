#include <iostream>
#include <unistd.h>
#include <signal.h>

#include "radio.hpp"

using namespace std ;

Radio *radio = nullptr ;

void my_handler(int s) {
	cout << "  Closing radio" << endl ; 
	delete radio ;
	exit( 0 ) ;
}

int main(int argc, char **argv, char **envp)
{
	// libusb_context *ctx ;
	// libusb_init( &ctx );
	// libusb_set_option( ctx, LIBUSB_OPTION_LOG_LEVEL, LIBUSB_LOG_LEVEL_WARNING);
   	
	signal( SIGINT, my_handler ) ;

	try {
		radio = new Radio() ;
		radio->start() ;

		std::cout << (*radio) << std::endl ;
		while( 1 ) { 
			sleep( 1 ) ;
		}

		delete radio ;
		radio = nullptr ;

	} catch ( const char *ex ) {
		delete radio ;
		radio = nullptr ;

		cerr << ex << endl ;
		return 1 ;
	}
	return 0;
}
