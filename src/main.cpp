#include<memory>
#include<chrono>
#include "common.hpp"
#include "net/TcpListener.hpp"
#include "net/TcpConnection.hpp"
#include "pool/BackendPool.hpp"
#include "balancer/RoundRobin.hpp"
#include "balancer/WeightedRR.hpp"
#include "balancer/Random.hpp"
#include "balancer/LeastConnections.hpp"
#include "balancer/IpHash.hpp"
#include "balancer/IAlgorithm.hpp"
#include "pool/Backend.hpp"
#include "config/ConfigLoader.hpp"
#include "health/HealthChecker.hpp"
#include "net/ConnectionRegistry.hpp"
#include "balancer/DrainMonitor.hpp"
#include "metrics/MetricsRegistry.hpp"
#include "control/ControlServer.hpp"

void function_client(int client_fd,BackendPool& pool, shared_ptr<IAlgorithm>algorithm,const string& client_ip,MetricsRegistry& registry) {
    auto start_session=chrono::steady_clock::now();
    sockaddr_in backend_address{};
    backend_address.sin_family=AF_INET;
    Backend* pick=algorithm->pick_backend(pool,client_ip);
    if (pick == nullptr) {
        cerr << "No healthy backend available\n";
        registry.record_error();
        auto end_session=chrono::steady_clock::now();
        auto latency = chrono::duration_cast<chrono::milliseconds>(end_session - start_session).count();
        registry.client_duration.record_latency(latency);
        return;
    }

    backend_address.sin_port=htons(pick->port);
    inet_pton(AF_INET,pick->host.c_str(),&backend_address.sin_addr);

    int backend_fd=socket(AF_INET,SOCK_STREAM,0);
    if(backend_fd<0) {
        perror("backend_socket");

        registry.record_error();

        auto end_session=chrono::steady_clock::now();
        auto latency = chrono::duration_cast<chrono::milliseconds>(end_session - start_session).count();
        registry.client_duration.record_latency(latency);

        return;
    }
    auto start_timer=chrono::steady_clock::now();

    int response=connect(backend_fd,(sockaddr*)&backend_address,sizeof(backend_address));

    if(response!=0) {
        perror("backend_connection");
        pick->health_machine.on_failure();
        close(backend_fd);
        registry.record_error();

        auto end_timer=chrono::steady_clock::now();
        auto latency_backend = chrono::duration_cast<chrono::milliseconds>(end_timer - start_timer).count();
        registry.backend_connection_latency.record_latency(latency_backend);

        auto end_session=chrono::steady_clock::now();
        auto latency = chrono::duration_cast<chrono::milliseconds>(end_session - start_session).count();
        registry.client_duration.record_latency(latency);

        return;
    }

    auto end_timer=chrono::steady_clock::now();
    auto latency_backend = chrono::duration_cast<chrono::milliseconds>(end_timer - start_timer).count();
    registry.backend_connection_latency.record_latency(latency_backend);

    registry.update_connection(false);

    pick->registry.register_connection(client_fd,backend_fd);
    pick->health_machine.on_success();
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
            pick->registry.unregister_connection(client_fd,backend_fd);

            registry.update_connection(true);
            registry.record_error();

            auto end_session=chrono::steady_clock::now();
            auto latency = chrono::duration_cast<chrono::milliseconds>(end_session - start_session).count();
            registry.client_duration.record_latency(latency);

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
            pick->registry.unregister_connection(client_fd,backend_fd);

            registry.update_connection(true);
            registry.record_error();

            auto end_session=chrono::steady_clock::now();
            auto latency = chrono::duration_cast<chrono::milliseconds>(end_session - start_session).count();
            registry.client_duration.record_latency(latency);

            return;
        }
        cout<<"Data received from backend"<<endl;
        cout<<"Data Sent to client"<<endl;
    }
    pick->active_connections--;
    pick->registry.unregister_connection(client_fd,backend_fd);

    registry.update_connection(true);

    auto end_session=chrono::steady_clock::now();
    auto latency = chrono::duration_cast<chrono::milliseconds>(end_session - start_session).count();
    registry.client_duration.record_latency(latency);
}

int main() {

    Config config=ConfigLoader::load("config/lb.toml");
    BackendPool pool;
    MetricsRegistry registry;

    for(auto backend: config.backends) {
        pool.add_backend(
            Backend(backend.host,backend.port,backend.weight)
        );
    }
    int drainout_time=config.drainout_time;
    
    mutex algorithm_mutex;
    shared_ptr<IAlgorithm>algorithm;

    DrainMonitor monitor(pool,drainout_time);
    monitor.start();

    HealthChecker checker(pool);
    checker.start();

    RoundRobin rr;
    WeightedRR wrr;
    Random rand;
    LeastConnections lc;
    IpHash ih;

    if(config.algorithm=="Round_Robin") {
        algorithm=make_shared<RoundRobin>();
    }else if(config.algorithm=="Weighted_Round_Robin") {
        algorithm=make_shared<WeightedRR>();
    }else if(config.algorithm=="Random") {
        algorithm=make_shared<Random>();
    }else if(config.algorithm=="Ip_Hash") {
        algorithm=make_shared<IpHash>();
    }else if(config.algorithm=="Least_Connections") {
        algorithm=make_shared<LeastConnections>();
    }

    ControlServer control("/tmp/lb-control.sock",ref(pool),"config/lb.toml",ref(algorithm),ref(algorithm_mutex),ref(monitor),ref(registry));
    control.start();

    TcpListener server;
    
    sockaddr_in address{};
    address.sin_family=AF_INET;
    address.sin_port=htons(config.listen_port);
    address.sin_addr.s_addr=INADDR_ANY;

    
    if(server.bind_to_port((sockaddr*)&address,sizeof(address))==-1) {
        perror("bind");
        monitor.stop();
        checker.stop();
        return 1;
    }

    cout<<"Server bound to port "<<ntohs(address.sin_port)<<endl;

    if(server.listen_on_port(2)==-1) {
        perror("listen");
        monitor.stop();
        checker.stop();
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
        registry.record_connection();
        char client_ip[INET_ADDRSTRLEN];

        inet_ntop(
            AF_INET,
            &client_address.sin_addr,
            client_ip,
            INET_ADDRSTRLEN
        );

        cout<<"Client Connected"<<endl;

        shared_ptr<IAlgorithm>selected_algorithm;
        {
            lock_guard<mutex>lock(algorithm_mutex);
            selected_algorithm=algorithm;
            cout << "Algorithm used for new connection: "
         << selected_algorithm->get_name() << endl;
        }

        

        thread t1(function_client,client_fd,ref(pool),selected_algorithm,client_ip,ref(registry));
        t1.detach();
    }

    monitor.stop();
    checker.stop();
    control.stop();
    return 0;
}