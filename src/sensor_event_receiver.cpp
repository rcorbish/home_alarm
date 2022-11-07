
#include <ctime>
#include <sstream>
#include <fstream>

#include "sensor_event_receiver.hpp"

using namespace std ;



SensorEventProcessor::SensorEventProcessor() {
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
                string device_name( line.begin()+i, line.end() ) ;
                sensors.emplace( device_id, device_name ) ;
                break ;
            }
        }

    }
    
}

void SensorEventProcessor::receive( const SensorEvent &event ) {
    
    auto stateItem = sensors.find( event.device_id ) ;
    if( stateItem != sensors.end() ) {
        SensorState &state = stateItem->second ;
        state.lastEvent =  time( nullptr ) ;

        bool changed = ( state.active != ( event.reed==1 ) ) ;
        changed |= state.low_battery != event.battery_low ;
        changed |= state.tamper != event.tamper ;

        if( changed ) {
            state.active = ( event.reed == 1 ) ;
            state.low_battery = event.battery_low ;
            state.tamper = event.tamper ;

            // cout << state << "\n" ;
            for( auto i : sensors ) {
                cout << i.second << "\n" ;
            }
            cout << endl ;
        }
    } else {
        cerr << event << "\n" ;
    }
}


std::ostream & operator << ( std::ostream &s, const SensorState &state ) {
    std::tm *now = localtime( &state.lastEvent ) ;

    char buf[80];
    strftime(buf, sizeof(buf), "%Y-%m-%d %X", now ) ;

    int n = state.device_name.size() ;
    n = ( n >= 28 ) ? 0 : ( 28 - n ) ;

    s << state.device_name ; 
    for( int i=0 ; i<n ; i++ )  s << " " ;

    s << buf
    << " State: " << state.active
    << " Lo-Batt " << state.low_battery
    << " Tamper " << state.tamper
    ;
    return s ;    
}
