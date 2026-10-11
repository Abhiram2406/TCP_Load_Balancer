#include<memory>
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

void function_client(int client_fd,BackendPool& pool, shared_ptr<IAlgorithm>algorithm,const string& client_ip) {
    sockaddr_in backend_address{};
    backend_address.sin_family=AF_INET;
    Backend* pick=algorithm->pick_backend(pool,client_ip);
    if (pick == nullptr) {
        cerr << "No healthy backend available\n";
        return;
    }

    backend_address.sin_port=htons(pick->port);
    inet_pton(AF_INET,pick->host.c_str(),&backend_address.sin_addr);

    int backend_fd=socket(AF_INET,SOCK_STREAM,0);
    if(backend_fd<0) {
        perror("backend_socket");
        return;
    }
    int response=connect(backend_fd,(sockaddr*)&backend_address,sizeof(backend_address));
    if(response!=0) {
        perror("backend_connection");
        pick->health_machine.on_failure();
        close(backend_fd);
        return;
    }
    pick->registry.register_connection(client_fd,backend_fd);
    cout<<pick->registry.get_active_count()<<endl;
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
            return;
        }
        cout<<"Data received from backend"<<endl;
        cout<<"Data Sent to client"<<endl;
    }
    pick->active_connections--;
    pick->registry.unregister_connection(client_fd,backend_fd);
}

void validate_inputs(BackendPool& pool,string toml_file,shared_ptr<IAlgorithm>algorithm,mutex& algorithm_mutex,DrainMonitor& monitor) {

    while(true) {
        string command;
        getline(cin,command);
        vector<string>command_tokens;
        string temp="";
        for(int i=0;i<command.length();i++) {
            if(command[i]==' ') {
                command_tokens.push_back(temp);
                temp="";
            }else{
                temp+=command[i];
            }
        }
        command_tokens.push_back(temp);
        if(!command_tokens.empty() && command_tokens[0]!="bob") {
            cout<<"INVALID COMMAND!!"<<endl;
            continue;
        }
        if(command_tokens.size()>2 && command_tokens[1]=="drain") {
            pool.drain_backend(command_tokens[2],stoi(command_tokens[3]));
            pool.list_backend();
        }else if(command_tokens.size()>1 && command_tokens[1]=="info") {
            pool.list_backend();
        }else if(command_tokens.size()>1 && command_tokens[1]=="reload") {
            Config new_config=ConfigLoader::load(toml_file);

            pool.update_backends(new_config.backends);
            cout<<"NEW BACKENDS WERE KEPT INTO IMPLEMENTATION!!"<<endl;
            monitor.update_drainout_time(new_config.drainout_time);
            cout<<"DRAINOUT TIME UPDATED!"<<endl;

            shared_ptr<IAlgorithm>new_algorithm;
            if(new_config.algorithm=="Round Robin") {
                new_algorithm=make_shared<RoundRobin>();
            }else if(new_config.algorithm=="Weighted Round Robin") {
                new_algorithm=make_shared<WeightedRR>();
            }else if(new_config.algorithm=="Random") {
                new_algorithm=make_shared<Random>();
            }else if(new_config.algorithm=="Ip Hash") {
                new_algorithm=make_shared<IpHash>();
            }else if(new_config.algorithm=="Least Connections") {
                new_algorithm=make_shared<LeastConnections>();
            }

            if(new_algorithm) {
                lock_guard<mutex>lock(algorithm_mutex);
                algorithm=new_algorithm;
                cout<<"ALGORITHM RELOADED SUCCESSFULLY"<<endl;
            }else{
                cout<<"INVALID ALGORITHM"<<endl;
            }

        }else{
            cout<<"INVALID COMMAND!!"<<endl;
            continue;
        }
    }
}

int main() {

    Config config=ConfigLoader::load("config/lb.toml");
    BackendPool pool;

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

    if(config.algorithm=="Round Robin") {
        algorithm=make_shared<RoundRobin>();
    }else if(config.algorithm=="Weighted Round Robin") {
        algorithm=make_shared<WeightedRR>();
    }else if(config.algorithm=="Random") {
        algorithm=make_shared<Random>();
    }else if(config.algorithm=="Ip Hash") {
        algorithm=make_shared<IpHash>();
    }else if(config.algorithm=="Least Connections") {
        algorithm=make_shared<LeastConnections>();
    }

    thread command_inputs(validate_inputs,ref(pool),"config/lb.toml",ref(algorithm),ref(algorithm_mutex),ref(monitor));
    command_inputs.detach();

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
        }

        

        thread t1(function_client,client_fd,ref(pool),selected_algorithm,client_ip);
        t1.detach();
    }

    monitor.stop();
    checker.stop();
    return 0;
}