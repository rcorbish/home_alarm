#include <iostream>
#include <unistd.h>

#include "radio.hpp"

using namespace std ;

int main(int argc, char **argv, char **envp)
{
	const char *fileName = argc>1 ? argv[1] : "test.csv" ;
	try {
		SignalProcessor dsp( 1000000 ) ;
		unsigned char *data = (unsigned char*)malloc( 0x40000 ) ;
		FILE *f = fopen( fileName, "r" ) ;
		if( !f ) {
			cerr << "Can't open test datafile [" << fileName << "]" << endl ;
			exit( 1 ) ;
		}
		int n=0 ;
		for( int i=0 ; i<0x40000 ; i++, n++ ) {
			uint16_t x ;
			if( !fscanf( f, "%ud", &x ) ) break ;
			data[i] = x ;
		}
		// uint32_t n = fread( data, 1, 0x40000, f ) ;
		fclose( f ) ;

		dsp.processRawBytes( data, n ) ;

		free( data ) ;
	} catch ( const char *ex ) {
		cerr << ex << endl ;
		return 1 ;
	}
	return 0;
}
