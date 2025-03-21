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
        int server_fd;
        sockaddr_in addr;
        int epoll_fd;
        epoll_event event
    };    

    server_data serv;

    server_tcp()
    {
        //trying create server
        
        if(try_create_server() == 1) throw 1;
    }

private:
    const int PORT = 8080;
    const int MAX_EVENTS = 1024;
    const int BUFFER_SIZE = 4096;
    
    // Установка неблокирующего режима для файлового дескриптора
    void set_nonblock(int fd) {
        int flags = fcntl(fd, F_GETFL, 0);
        fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    }

    bool create_tcp_socet()
    {
        serv.server_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
        if (serv.server_fd == -1) {
            std::cerr << "socket() failed: " << strerror(errno) << std::endl;
            return true;
        }
        return false;
    }

    void settings_addr()
    {
        serv.addr.sin_family = AF_INET;
        serv.addr.sin_addr.s_addr = INADDR_ANY;
        serv.addr.sin_port = htons(PORT);
    }

    bool connect_tcp_scoket()
    {
        if (bind(serv.server_fd, (sockaddr*)&serv.addr, sizeof(serv.addr)) == -1) {
            std::cerr << "bind() failed: " << strerror(errno) << std::endl;
            close(serv.server_fd);
            return true;
        }
        return false;
    }

    bool switch_to_listening_mode()
    {
        if (listen(serv.server_fd, SOMAXCONN) == -1) {
            std::cerr << "listen() failed: " << strerror(errno) << std::endl;
            close(serv.server_fd);
            return true;
        }
        return false;
    }
    
    bool create_epoll()
    {
        serv.epoll_fd = epoll_create1(0);
        if (serv.epoll_fd == -1) {
            std::cerr << "epoll_create() failed: " << strerror(errno) << std::endl;
            close(serv.server_fd);
            return true;
        }
        return false;
    }

    bool add_socket_to_epoll()
    {
        serv.event.data.fd = serv.server_fd;
        serv.event.events = EPOLLIN | EPOLLET; // Edge-Triggered режим
        if (epoll_ctl(serv.epoll_fd, EPOLL_CTL_ADD, serv.server_fd, &serv.event) == -1) {
            std::cerr << "epoll_ctl() failed: " << strerror(errno) << std::endl;
            close(serv.server_fd);
            close(serv.epoll_fd);
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


        std::vector<epoll_event> events(MAX_EVENTS);
        std::cout << "Server started on port " << PORT << std::endl;

        server_listening();

        return 0;
    }

    void server_listening()
    {
        while (true) {
            int num_events = epoll_wait(epoll_fd, events.data(), events.size(), -1);
            if (num_events == -1) {
                if (errno == EINTR) continue; // Перезапуск при прерывании сигналом
                std::cerr << "epoll_wait() failed: " << strerror(errno) << std::endl;
                break;
            }

            for (int i = 0; i < num_events; ++i) {
                if (events[i].data.fd == server_fd) {
                    // Обработка новых подключений
                    while (true) {
                        sockaddr_in client_addr{};
                        socklen_t addr_len = sizeof(client_addr);
                        int client_fd = accept4(server_fd, (sockaddr*)&client_addr, 
                                            &addr_len, SOCK_NONBLOCK);
                        if (client_fd == -1) {
                            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                            std::cerr << "accept() failed: " << strerror(errno) << std::endl;
                            break;
                        }

                        // Добавление клиента в epoll
                        event.data.fd = client_fd;
                        event.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
                        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event) == -1) {
                            std::cerr << "epoll_ctl(client) failed: " << strerror(errno) << std::endl;
                            close(client_fd);
                        }
                    }
                } else {
                    // Обработка данных от клиентов
                    if (events[i].events & EPOLLRDHUP) {
                        // Клиент отключился
                        close(events[i].data.fd);
                        continue;
                    }

                    if (events[i].events & EPOLLIN) {
                        char buffer[BUFFER_SIZE];
                        ssize_t bytes_read;
                        
                        while ((bytes_read = read(events[i].data.fd, buffer, sizeof(buffer)))) {
                            if (bytes_read == -1) {
                                if (errno != EAGAIN) {
                                    std::cerr << "read() error: " << strerror(errno) << std::endl;
                                    close(events[i].data.fd);
                                }
                                break;
                            }

                            // Эхо-ответ
                            write(events[i].data.fd, buffer, bytes_read);
                        }
                    }
                }
            }
        }
        close(server_fd);
        close(epoll_fd);
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