#pragma once
#include "../common.hpp"
class TcpListener {
    private:
    int server_fd;
    public:
    TcpListener ();
    ~TcpListener ();
    int bind_to_port(const sockaddr* address,socklen_t length);
    int listen_on_port(int max_queue);
    int accept_client(sockaddr* address,socklen_t* length);
    int get_fd();
};