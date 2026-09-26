#include "TcpConnection.hpp"

TcpConnection::TcpConnection(int fd) : client_fd(fd) {}
TcpConnection::~TcpConnection() {
    close(client_fd);
}
ssize_t TcpConnection:: receive_data( char* buffer,size_t length) {
    ssize_t received_data=recv(client_fd,buffer,length,0);
    return received_data;
}
ssize_t TcpConnection:: send_data(const char* buffer,size_t length) {
    ssize_t sent_data=send(client_fd,buffer,length,0);
    return sent_data;
}