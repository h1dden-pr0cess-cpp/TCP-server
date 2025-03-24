#ifndef _CLIENT_TCP_H_
#define _CLIENT_TCP_H_

#include "client_tcp.h"
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <vector>
#include <thread>
class client_tcp{
    public:
    client_tcp();
    void create_client();
    private: 
    struct client_data {
        const int MAX_EVENTS = 10;
        const int PORT = 8080;
        const char* SERVER_IP = "127.0.0.1";
        int socket_fd;
        sockaddr_in serv_addr;
        int epoll_fd;
        epoll_event event;
        char buffer[1024];
        bool running = true;
    };

    client_data sch;

    void set_nonblock(int fd);
    bool create_client_socket();
    void settings_address();
    bool add_inet_pton();
    bool connect_to_the_server();
    bool create_epoll();
    void register_socket_and_stdin_at_epoll();
    void monitoring_socket();
    void monitoring_stdin();
    void client_connect_to_the_server();
    int try_create_client();
    
};


#endif