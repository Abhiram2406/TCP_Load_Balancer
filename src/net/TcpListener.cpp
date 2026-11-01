#include "TcpListener.hpp"

TcpListener::TcpListener() {
    int fd=socket(AF_INET,SOCK_STREAM,0);
    server_fd=fd;
}
TcpListener::~TcpListener() {
    close(server_fd);
}
int TcpListener::bind_to_port(const sockaddr* address,socklen_t length) {
    int result=bind(server_fd,address,length);
    return result;
}
int TcpListener::listen_on_port(int max_queue) {
    int result=listen(server_fd,max_queue);
    return result;
}
int TcpListener::accept_client(sockaddr* address,socklen_t* length) {
    int client_fd=accept(server_fd,address,length);
    return client_fd;
}
int TcpListener::get_fd() {
    return server_fd;
}