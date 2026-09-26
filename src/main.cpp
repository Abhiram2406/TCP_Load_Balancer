#include "common.hpp"
#include "net/TcpListener.hpp"
#include "net/TcpConnection.hpp"

void function_client(int client_fd) {
    sockaddr_in backend_address{};
    backend_address.sin_family=AF_INET;
    backend_address.sin_port=htons(9000);
    inet_pton(AF_INET,"127.0.0.1",&backend_address.sin_addr);

    int backend_fd=socket(AF_INET,SOCK_STREAM,0);
    if(backend_fd<0) {
        perror("backend_socket");
        return;
    }
    if(connect(backend_fd,(sockaddr*)&backend_address,sizeof(backend_address))==-1) {
        perror("backend_connection");
        close(backend_fd);
        return;
    }
    TcpConnection backend(backend_fd);
    TcpConnection client(client_fd);

    char buffer[1024];

    ssize_t received_data_client=client.receive_data(buffer,sizeof(buffer));
    ssize_t send_data_backend=backend.send_data(buffer,received_data_client);

    if(received_data_client<0 || send_data_backend<0) {
        perror("receive");
        return;
    }

    cout<<"Received data from Client: "<<endl;
    cout<<"Sent data to backend: "<<endl;
    cout.write(buffer,received_data_client);
    
    ssize_t receive_data_backend=backend.receive_data(buffer,sizeof(buffer));
    ssize_t send_data_client=client.send_data(buffer,received_data_client);

    if(send_data_client<0 || receive_data_backend<0) {
        perror("send");
        return;
    }
    cout<<"Data received from backend"<<endl;
    cout<<"Data Sent to client"<<endl;
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