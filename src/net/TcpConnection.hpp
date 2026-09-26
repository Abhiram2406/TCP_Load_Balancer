#pragma once
#include "../common.hpp"
class TcpConnection {
    private:
    int client_fd;
    public:
    TcpConnection (int fd);
    ~TcpConnection ();
    ssize_t send_data(const char* st,size_t length);
    ssize_t receive_data(char *st,size_t length);
};