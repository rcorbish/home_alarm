#pragma once

#include "sensors.hpp"

constexpr int DEFAULT_PORT = 9111 ;

class Listener {
    private:
        int server_fd ;
        pthread_t thread_id ;
        Sensors &sensors;
        
    protected:
        static void* receiver_loop( void *arg) ;

    public:
        Listener( Sensors &sensors, const int port = DEFAULT_PORT) ;
        void start() ;
} ;