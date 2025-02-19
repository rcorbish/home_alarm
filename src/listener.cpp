
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "listener.hpp"

#define BUFFER_SIZE 1024

Listener::Listener( Sensors &_sensors, const int port ) : sensors(_sensors) {
    
    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    
    // Set socket options to reuse address
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    
    struct sockaddr_in address;
    // Setup server address structure
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    
    // Bind the socket to the network address and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    
    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    
    std::cout << "Server listening on port " << port << std::endl;
}

void Listener::start() {
    // Create listener thread
    if (pthread_create(&thread_id, nullptr, receiver_loop, this) != 0) {
        perror("pthread_create failed");
        exit(EXIT_FAILURE);
    }
}

void* Listener::receiver_loop( void *arg ) {
    Listener *self = static_cast<Listener *>(arg);

    char buffer[BUFFER_SIZE] = {0};

    while(true) {

        fd_set readfds;
        struct timeval timeout;
        FD_ZERO(&readfds);
        FD_SET(self->server_fd, &readfds);
        timeout.tv_sec = 1;  // 1 second timeout
        timeout.tv_usec = 0;
        
        int activity = select(self->server_fd + 1, &readfds, NULL, NULL, &timeout);
        
        if (activity < 0 && errno != EINTR) {
            perror("select error");
            continue;
        }
        
        // If timeout occurred, just loop back to check server_running
        if (activity == 0) {
            continue;
        }

        int new_socket;

        struct sockaddr_in address;
        const int addrlen = sizeof(address);
    
        // Accept incoming connection
        if ((new_socket = accept(self->server_fd, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }
        
        std::cout << "Client connected" << std::endl;
        
        // Read client request
        int valread = read(new_socket, buffer, BUFFER_SIZE);
        std::cout << "Received request: " << buffer << std::endl;
        
        // Process request (simple echo in this example)
        auto response = self->sensors.toString();
        
        // Send response back to client
        send(new_socket, response.c_str(), response.length(), 0);
        std::cout << "Response sent" << std::endl;
        
        // Close connection
        close(new_socket);
    }    
    return nullptr;
}