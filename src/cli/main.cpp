#include "../common.hpp"
using namespace std;

void socket_communication(string command) {
    int client_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (client_fd < 0) {
        perror("socket");
        return;
    }
    sockaddr_un address{};
    address.sun_family = AF_UNIX;
    strncpy(address.sun_path,"/tmp/lb-control.sock",sizeof(address.sun_path) - 1);
    if (connect(client_fd,(sockaddr*)&address,sizeof(address)) < 0) {
        perror("connect");
        close(client_fd);
        return;
    }
    command += "\n";
    if (send(client_fd, command.c_str(), command.size(), 0) < 0) {
        perror("send");
        close(client_fd);
        return;
    }
    shutdown(client_fd, SHUT_WR);
    char buffer[4096];
    ssize_t n;
    while ((n = recv(client_fd, buffer, sizeof(buffer), 0)) > 0) {
        cout.write(buffer, n);
    }
    if (n < 0) {
        perror("recv");
        close(client_fd);
        return;
    }
    close(client_fd);
}

int main(int argc, char* argv[]) {
    if(argc>=2) {
        string command = argv[1];
        for (int i = 2; i < argc; i++) {
            command += " ";
            command += argv[i];
        }
        socket_communication(command);
    }else{
        while(true) {
            string command;
            getline(cin,command);
            if(command=="exit") {
                break;
            }
            socket_communication(command);
        }
    }
    return 0;
}