#include <iostream>
#include <unistd.h>
#include <signal.h>
#include <string>
#include <vector>
#include <limits>

#include "radio.hpp"
#include "listener.hpp"
#include "signal_processor.hpp"

using namespace std ;

vector<string> split(const string& str, char delimiter) ;

Listener *listener = nullptr ;

void my_handler(int s) {
	cout << "  Closing application" << endl ; 
	delete listener ;
	exit( 0 ) ;
}

int main( int argc, char **argv, char **envp ) {
   	
	signal( SIGINT, my_handler ) ;

	try {
		listener = new Listener() ;
		listener->start() ;
		string line;
		cin.ignore(numeric_limits<streamsize>::max(), '\n');

		while (getline(cin, line)) {
			auto tokens = split(line, ',');
			// cout << line << endl ;
			if( line[0] == 'i' && line[1] == 'd' ) {		// skip headers
				continue ;
			}
			if (tokens.size() < 8) {	
				cerr << "Too few fields: " << line << endl ;
				continue ;
			}
			try {
				SensorEvent event {
					.device_id = (uint32_t)stol(tokens[0]),
					.contact = tokens[4] == "1",
					.tamper = tokens[3] == "1",
					.reed = tokens[5] == "1",
					.alarm = tokens[6] == "1",
					.battery_low = tokens[1] == "0",
					.heartbeat = tokens[7] == "1"
				};
				listener->acceptEvent(event);
				// cout << event.device_id << endl ;
			} catch (const invalid_argument& e) {
				cerr << "Invalid argument: " << e.what() << endl ;
			}
		}

	} catch ( const char *ex ) {
		cerr << ex << endl ;
		return 1 ;
	}
	return 0;
}

vector<string> split(const string& str, char delimiter) {
    std::vector<string> result;
    size_t start = 0;
    size_t end = str.find(delimiter);
    
    while (end != string::npos) {
        result.push_back(str.substr(start, end - start));
        start = end + 1;
        end = str.find(delimiter, start);
    }
    
    // Don't forget the last part after the last delimiter
    result.push_back(str.substr(start));
    
    return result;
}
