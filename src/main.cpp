#include <sys/socket.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
#include <cstring>
#include <vector>
#include <cerrno>



class server_tcp{
public:
    

struct server_data{
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

    server_tcp()
    {
        //trying create server
        if(try_create_server() == 1) throw 1;
    }

private:

    
    // Установка неблокирующего режима для файлового дескриптора
    void set_nonblock(int fd) {
        int flags = fcntl(fd, F_GETFL, 0);
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }

    bool create_tcp_socet()
    {
        sch.server_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
        if (sch.server_fd == -1) {
            std::cerr << "socket() failed: " << strerror(errno) << std::endl;
            return true;
        }
        return false;
    }

    void settings_addr()
    {
        sch.addr.sin_family = AF_INET;
        sch.addr.sin_addr.s_addr = INADDR_ANY;
        sch.addr.sin_port = htons(sch.PORT);
    }

    bool connect_tcp_scoket()
    {
        if (bind(sch.server_fd, (sockaddr*)&sch.addr, sizeof(sch.addr)) == -1) {
            std::cerr << "bind() failed: " << strerror(errno) << std::endl;
            close(sch.server_fd);
            return true;
        }
        return false;
    }

    bool switch_to_listening_mode()
    {
        if (listen(sch.server_fd, SOMAXCONN) == -1) {
            std::cerr << "listen() failed: " << strerror(errno) << std::endl;
            close(sch.server_fd);
            return true;
        }
        return false;
    }
    
    bool create_epoll()
    {
        sch.epoll_fd = epoll_create1(0);
        if (sch.epoll_fd == -1) {
            std::cerr << "epoll_create() failed: " << strerror(errno) << std::endl;
            close(sch.server_fd);
            return true;
        }
        return false;
    }

    bool add_socket_to_epoll()
    {
        sch.event.data.fd = sch.server_fd;
        sch.event.events = EPOLLIN | EPOLLET; // Edge-Triggered режим
        if (epoll_ctl(sch.epoll_fd, EPOLL_CTL_ADD, sch.server_fd, &sch.event) == -1) {
            std::cerr << "epoll_ctl() failed: " << strerror(errno) << std::endl;
            close(sch.server_fd);
            close(sch.epoll_fd);
            return true;
        }
        return false;
    }

    int try_create_server()
    {
        // Создание TCP-сокета
        if(create_tcp_socet()) return 1;

        // Настройка адреса сервера
        settings_addr();

        // Привязка сокета
        if(connect_tcp_scoket()) return 1;

        // Переход в режим прослушивания
        if(switch_to_listening_mode()) return 1;
       
        // Создание epoll-инстанса
        if(create_epoll()) return 1;

        // Добавление серверного сокета в epoll
        if(add_socket_to_epoll()) return 1;
        
        std::cout << "Server started on port " << sch.PORT << std::endl;

        server_listening();

        return 0;
    }

    void server_listening()
    {
        while (true) {
            int num_events = epoll_wait(sch.epoll_fd, sch.events.data(), sch.events.size(), -1);
            if (num_events == -1) {
                if (errno == EINTR) continue; // Перезапуск при прерывании сигналом
                std::cerr << "epoll_wait() failed: " << strerror(errno) << std::endl;
                break;
            }

            for (int i = 0; i < num_events; ++i) {
                if (sch.events[i].data.fd == sch.server_fd) {
                    // Обработка новых подключений
                    while (true) {
                        sockaddr_in client_addr{};
                        socklen_t addr_len = sizeof(client_addr);
                        int client_fd = accept4(sch.server_fd, (sockaddr*)&client_addr, 
                                            &addr_len, SOCK_NONBLOCK);
                        if (client_fd == -1) {
                            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                            std::cerr << "accept() failed: " << strerror(errno) << std::endl;
                            break;
                        }

                        // Добавление клиента в epoll
                        sch.event.data.fd = client_fd;
                        sch.event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
                        if (epoll_ctl(sch.epoll_fd, EPOLL_CTL_ADD, client_fd, &sch.event) == -1) {
                            std::cerr << "epoll_ctl(client) failed: " << strerror(errno) << std::endl;
                            close(client_fd);
                        }
                    }
                } else {
                    // Обработка данных от клиентов
                    if (sch.events[i].events & EPOLLRDHUP) {
                        // Клиент отключился
                        close(sch.events[i].data.fd);
                        continue;
                    }

                    if (sch.events[i].events & EPOLLIN) {
                        char buffer[sch.BUFFER_SIZE];
                        ssize_t bytes_read;
                        
                        while ((bytes_read = read(sch.events[i].data.fd, buffer, sizeof(buffer)))) {
                            if (bytes_read == -1) {
                                if (errno != EAGAIN) {
                                    std::cerr << "read() error: " << strerror(errno) << std::endl;
                                    close(sch.events[i].data.fd);
                                }
                                break;
                            }

                            // Эхо-ответ
                            write(sch.events[i].data.fd, buffer, bytes_read);
                        }
                    }
                }
            }
        }
        close(sch.server_fd);
        close(sch.epoll_fd);
    }

};




int main() {

    try{
        server_tcp server;
    }
    catch(const int error)
    {
        std::cerr << "Error to creat server: Code error" << error <<std::endl;
        return 1;
    }
    return 0;
}