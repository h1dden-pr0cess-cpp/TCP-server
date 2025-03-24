// server_tcp.h
#ifndef _SERVER_TCP_H_
#define _SERVER_TCP_H_

#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <cstring>
#include <vector>
#include <cerrno>
#include <thread>

class server_tcp {
public:
    server_tcp();
    void create_server();

private:
    struct server_data {
        static constexpr int PORT = 8080;
        static constexpr int MAX_EVENTS = 1024;
        static constexpr int BUFFER_SIZE = 4096;

        int server_fd;
        sockaddr_in addr;
        int epoll_fd;
        epoll_event event;
        std::vector<epoll_event> events{MAX_EVENTS};
    };
    server_data sch;
    
    void set_nonblock(int fd);
    bool create_tcp_socket();
    void settings_addr();
    bool connect_tcp_socket();
    bool switch_to_listening_mode();
    bool create_epoll();
    bool add_socket_to_epoll();
    int try_create_server();
    void server_listening();
};

#endif // _SERVER_TCP_H_