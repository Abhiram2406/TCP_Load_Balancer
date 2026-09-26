#include "common.hpp"
#include "net/TcpListener.hpp"
#include "net/TcpConnection.hpp"

void function_client(int client_fd) {
    TcpConnection client(client_fd);

    char buffer[1024];

    ssize_t received_data=client.receive_data(buffer,sizeof(buffer));

    if(received_data<0) {
        perror("receive");
        return;
    }

    cout<<"Received data from Client: ";
    cout.write(buffer,received_data);

    ssize_t send_data=client.send_data(buffer,received_data);

    if(send_data<0) {
        perror("send");
        return;
    }
    cout<<"Data Sent to client: "<<endl;
}

int main() {
    TcpListener server;
    
    sockaddr_in address{};
    address.sin_family=AF_INET;
    address.sin_port=htons(2424);
    address.sin_addr.s_addr=INADDR_ANY;
    
    if(server.bind_to_port((sockaddr*)&address,sizeof(address))==-1) {
        perror("bind");
        return 1;
    }

    cout<<"Server bound to port "<<ntohs(address.sin_port)<<endl;

    if(server.listen_on_port(2)==-1) {
        perror("listen");
        return 1;
    }

    cout<<"listening on port"<<ntohs(address.sin_port)<<endl;

    while(true) {
        sockaddr_in client_address{};
        socklen_t client_address_length=sizeof(client_address);

        int client_fd=server.accept_client((sockaddr*)&client_address,&client_address_length);

        if(client_fd==-1) {
            perror("accept");
            continue;
        }

        cout<<"Client Connected"<<endl;

        thread t1(function_client,client_fd);
        t1.detach();
    }

    return 0;
}