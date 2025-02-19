
#include <ctime>
#include <sstream>
#include <fstream>
#include <string.h>

#include "sensors.hpp"

using namespace std ;



Sensors::Sensors() {
    ifstream sensorNames( "sensors.txt" ) ;
    string line;
    while( getline( sensorNames, line ) ) {
        for (auto i = 0; i < line.size(); i++) {
            if( isspace( line[i] ) ) { 
                string device_id_name( line.begin(), line.begin()+i ) ;
                stringstream ss( device_id_name ) ;
                ss << hex ;
                int device_id ;                
                ss >> device_id ;

                while( isspace( line[i] ) ) { 
                    i++ ;
                }
                char device_type = line[i] ;
                while( !isspace( line[i] ) ) { 
                    i++ ;
                }

                while( isspace( line[i] ) ) { 
                    i++ ;
                }
                string device_name( line.begin()+i, line.end() ) ;
                sensors.emplace( piecewise_construct,
                                 forward_as_tuple(device_id), 
                                 forward_as_tuple(device_name, device_type) ) ;
                break ;
            }
        }
    }
}

void Sensors::accept( const SensorEvent &event ) {    
    auto stateItem = sensors.find( event.device_id ) ;
    if( stateItem != sensors.end() ) {
        SensorState &state = stateItem->second ;
        state.active = state.device_type=='C' ? event.contact : event.reed ;
        if( state.active ) {
            state.lastEvent =  time( nullptr ) ;
        }
        state.low_battery = event.battery_low ;
        state.tamper = event.tamper ;

        // cout << state << "\n" ;
        cout << toString() << endl ;
    } else {
//        cerr << event << "\n" ;
    }
}

std::string Sensors::toString() const {
    stringstream ss ;
    for( auto i : sensors ) {
        ss << i.second << "\n" ;
    }
    return ss.str() ;
}

std::ostream & operator << ( std::ostream &s, const SensorState &state ) {

    s << "{ \"name\": \"state.device_name\"," 
      << " \"active\": " << state.active
      << " \"low_battery\": " << state.low_battery
      << " \"tamper\": " << state.tamper
      << " \"lastEvent\": " << state.lastEvent
      << " }" ;

    return s ;    
}
