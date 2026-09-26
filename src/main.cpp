#include "common.hpp"

void function_client(int client_fd) {

    char buffer[1024];

    ssize_t received_data=recv(client_fd,buffer,sizeof(buffer),0);

    if(received_data<0) {
        perror("receive");
        close(client_fd);
        return;
    }

    cout<<"Received";
    cout.write(buffer,received_data);

    ssize_t send_data=send(client_fd,buffer,received_data,0);

    if(send_data<0) {
        perror("send");
        close(client_fd);
        return;
    }
    cout<<"Data Sent to client"<<endl;
    close(client_fd);
}

int main() {
    int server_fd=socket(AF_INET,SOCK_STREAM,0);
    
    sockaddr_in address{};
    address.sin_family=AF_INET;
    address.sin_port=htons(2424);
    address.sin_addr.s_addr=INADDR_ANY;

    if(bind(server_fd,(sockaddr*)&address,sizeof(address))==-1) {
        perror("bind");
        close(server_fd);
        return 1;
    }

    cout<<"Server bound to"<<ntohs(address.sin_port)<<endl;

    if(listen(server_fd,2)==-1) {
        perror("listen");
        close(server_fd);
        return 1;
    }

    cout<<"listening on port"<<ntohs(address.sin_port)<<endl;

    while(true) {
        sockaddr_in client_address{};
        socklen_t client_address_length=sizeof(client_address);

        int client_fd=accept(server_fd,(sockaddr*)&client_address,&client_address_length);

        if(client_fd==-1) {
            perror("accept");
            close(server_fd);
            continue;
        }

        cout<<"Client Connected"<<endl;

        thread t1(function_client,client_fd);
        t1.detach();
    }

    return 0;
}