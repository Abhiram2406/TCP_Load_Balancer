#include "common.hpp"
#include "net/TcpListener.hpp"
#include "net/TcpConnection.hpp"
#include "pool/BackendPool.hpp"
#include "balancer/RoundRobin.hpp"
#include "balancer/WeightedRR.hpp"
#include "balancer/Random.hpp"
#include "balancer/LeastConnections.hpp"
#include "balancer/IAlgorithm.hpp"
#include "pool/Backend.hpp"

void function_client(int client_fd,BackendPool& pool, IAlgorithm& algorithm) {
    sockaddr_in backend_address{};
    backend_address.sin_family=AF_INET;
    Backend* pick=algorithm.pick_backend(pool);
    if (pick == nullptr) {
        cerr << "No healthy backend available\n";
        return;
    }
    
    cout << "Selected backend: "
     << pick->port
     << " | active: "
     << pick->active_connections
     << endl;

    backend_address.sin_port=htons(pick->port);
    inet_pton(AF_INET,pick->host.c_str(),&backend_address.sin_addr);

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
    pick->active_connections++;
    TcpConnection backend(backend_fd);
    TcpConnection client(client_fd);

    char buffer[1024];

    while(true) {
        ssize_t received_data_client=client.receive_data(buffer,sizeof(buffer));
        if(received_data_client<=0) {
            break;
        }
        ssize_t send_data_backend=backend.send_data(buffer,received_data_client);

        if(send_data_backend<0) {
            perror("receive");
            pick->active_connections--;
            return;
        }

        cout<<"Received data from Client: "<<endl;
        cout<<"Sent data to backend: "<<endl;
        cout.write(buffer,received_data_client);
        
        ssize_t receive_data_backend=backend.receive_data(buffer,sizeof(buffer));
        if(receive_data_backend<=0) {
            break;
        }
        ssize_t send_data_client=client.send_data(buffer,receive_data_backend);

        if(send_data_client<0) {
            perror("send");
            pick->active_connections--;
            return;
        }
        cout<<"Data received from backend"<<endl;
        cout<<"Data Sent to client"<<endl;
    }
    pick->active_connections--;
}

int main() {

    BackendPool pool;
    Backend B1("127.0.0.1",9000,3);
    Backend B2("127.0.0.1",9001,1);
    Backend B3("127.0.0.1",9002,2);
    Backend B4("127.0.0.1",9003,2);
    Backend B5("127.0.0.1",9004,2);
    pool.add_backend(B1);
    pool.add_backend(B2);
    pool.add_backend(B3);
    // pool.add_backend(B4);
    // pool.add_backend(B5);

    LeastConnections rr;


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

        thread t1(function_client,client_fd,ref(pool),ref(rr));
        t1.detach();
    }

    return 0;
}