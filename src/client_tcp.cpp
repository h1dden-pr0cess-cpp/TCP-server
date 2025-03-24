#include "client_tcp.h"

client_tcp::client_tcp() {
    create_client();
}

void client_tcp::create_client() {
    if (try_create_client() == 1) throw 1;
}

void client_tcp::set_nonblock(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

bool client_tcp::create_client_socket() {
    sch.socket_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (sch.socket_fd < 0) {
        perror("socket");
        return true;
    }
    return false;
}

void client_tcp::settings_address() {
    sch.serv_addr.sin_family = AF_INET;
    sch.serv_addr.sin_port = htons(sch.PORT);
}

bool client_tcp::add_inet_pton() {
    if (inet_pton(AF_INET, sch.SERVER_IP, &sch.serv_addr.sin_addr) <= 0) {
        perror("inet_pton");
        close(sch.socket_fd);
        return true;
    }
    return false;
}

bool client_tcp::connect_to_the_server() {
    if (connect(sch.socket_fd, (sockaddr*)&sch.serv_addr, sizeof(sch.serv_addr)) < 0) {
        if (errno != EINPROGRESS) {
            perror("connect");
            close(sch.socket_fd);
            return true;
        }
    }
    return false;
}

bool client_tcp::create_epoll() {
    sch.epoll_fd = epoll_create1(0);
    if (sch.epoll_fd < 0) {
        perror("epoll_create");
        close(sch.socket_fd);
        return true;
    }
    return false;
}

void client_tcp::register_socket_and_stdin_at_epoll() {
    sch.event.events = EPOLLIN | EPOLLOUT | EPOLLET;
}

void client_tcp::monitoring_socket() {
    sch.event.data.fd = sch.socket_fd;
    epoll_ctl(sch.epoll_fd, EPOLL_CTL_ADD, sch.socket_fd, &sch.event);
}

void client_tcp::monitoring_stdin() {
    set_nonblock(STDIN_FILENO);
    sch.event.data.fd = STDIN_FILENO;
    epoll_ctl(sch.epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &sch.event);
}

void client_tcp::client_connect_to_the_server() {
    while (sch.running) {
        epoll_event events[sch.MAX_EVENTS];
        int n = epoll_wait(sch.epoll_fd, events, sch.MAX_EVENTS, -1);

        for (int i = 0; i < n; ++i) {
            if (events[i].data.fd == STDIN_FILENO) {
                // Обработка ввода пользователя
                ssize_t bytes = read(STDIN_FILENO, sch.buffer, sizeof(sch.buffer));
                if (bytes > 0) {
                    send(sch.socket_fd, sch.buffer, bytes, 0);
                } else if (bytes == 0) {
                    sch.running = false;
                }
            } else if (events[i].data.fd == sch.socket_fd) {
                // Обработка данных от сервера
                ssize_t bytes = recv(sch.socket_fd, sch.buffer, sizeof(sch.buffer), 0);
                if (bytes > 0) {
                    std::cout << "Server response: ";
                    std::cout.write(sch.buffer, bytes);
                } else if (bytes == 0) {
                    std::cout << "Connection closed by server\n";
                    sch.running = false;
                }
            }
        }
    }
}

int client_tcp::try_create_client() {
    // Создание клиентского сокета
    if (create_client_socket()) return 1;

    // Настройка адреса сервера
    settings_address();
    if (add_inet_pton()) return 1;

    // Подключение к серверу
    if (connect_to_the_server()) return 1;

    // Создание epoll
    if (create_epoll()) return 1;

    // Регистрация сокета и stdin в epoll
    register_socket_and_stdin_at_epoll();

    // Мониторинг сокета
    monitoring_socket();

    // Мониторинг stdin
    monitoring_stdin();

    std::cout << "Connected to server. Type messages (Ctrl+D to exit):\n";
    client_connect_to_the_server();

    close(sch.socket_fd);
    close(sch.epoll_fd);

    return 0; 
}