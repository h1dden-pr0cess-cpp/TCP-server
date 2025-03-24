#include "server_tcp.h"
#include "client_tcp.h"

bool function_create_server()
{
    server_tcp server;
    try{
        server.create_server();
    }
    catch(const bool error)
    {
        std::cerr << "Error to creat server: Code error" << error <<std::endl;
        return true;
    }
    return false;
}

bool function_create_client()
{
    client_tcp client;
    try{
        client.create_client();
    }
    catch(const bool error)
    {
        std::cerr << "Error connect to the server: Code error" << error <<std::endl;
        return true;
    }
    return false;
}

bool menu()
{
    std::cout << "Create server: press 0" << std::endl;
    std::cout << "Connect to the server: press 1" << std::endl;
    short temp_choice;
    std::cin >> temp_choice;
    switch (temp_choice)
    {
    case 0:
        if(function_create_server()) return true;
        break;
    case 1:
        if(function_create_client()) return true;
        break;
    default:
        break;
    }
    return false;
}

int main() {

    if(menu()) return 1; 
    
    return 0;
}