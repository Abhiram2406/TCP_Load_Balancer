#include "ControlServer.hpp"
#include "../config/ConfigLoader.hpp"
#include <sstream>
ControlServer::ControlServer(string path,BackendPool& p,string file,shared_ptr<IAlgorithm>a,mutex& am,DrainMonitor& m,MetricsRegistry& r):pool(p),socket_path(path),toml_file(file),algorithm(a),algorithm_mutex(am),monitor(m),registry(r) {}

ControlServer::~ControlServer() {
    stop();
}

void ControlServer::start() {
    server_thread = thread([this]() {
            server_fd = socket(AF_UNIX, SOCK_STREAM, 0);
            if (server_fd < 0) {
                perror("control/socket");
                return;
            }
            sockaddr_un address{};
            address.sun_family = AF_UNIX;
            strncpy(address.sun_path,socket_path.c_str(),sizeof(address.sun_path) - 1);
            unlink(socket_path.c_str());
            if (bind(server_fd,(sockaddr*)&address,sizeof(address)) < 0) {
                perror("control/bind");
                cerr << "Socket path was: [" << socket_path << "]\n";
                cerr << "Path length: " << socket_path.size() << '\n';
                close(server_fd);
                server_fd = -1;
                return;
            }
            if (listen(server_fd, 10) < 0) {
                perror("control/listen");
                close(server_fd);
                server_fd = -1;
                unlink(socket_path.c_str());
                return;
            }
            running = true;

        while (running) {
            int client_fd = accept(server_fd, nullptr, nullptr);
            if (client_fd < 0) {
                if (!running) break;
                continue;
            }
            handle_client(client_fd);
            close(client_fd);
        }

        close(server_fd);
        unlink(socket_path.c_str());
    });
}
void ControlServer::stop() {
    running=false;
    int wake_fd=socket(AF_UNIX,SOCK_STREAM,0);
    if(wake_fd<0) {
        perror("accept_stopping socket");
        return;
    }
    sockaddr_un address{};
    address.sun_family=AF_UNIX;
    strncpy(address.sun_path,socket_path.c_str(),sizeof(address.sun_path) - 1);
    connect(wake_fd,(sockaddr*)&address,sizeof(address));
    close(wake_fd);
    if(server_thread.joinable()) {
        server_thread.join();
    }
}
void ControlServer::handle_client(int client_fd) {
    char buffer[1096];
    while(true) {
        size_t n=recv(client_fd,buffer,sizeof(buffer)-1,0);
        if(n<=0) {
            return;
        }
        buffer[n]='\0';
        string command(buffer);
        while(!command.empty() && (command.back()=='\n' || command.back()=='\r')) {
            command.pop_back();
        }
        string response=process_command(command)+'\n';
        if (send(client_fd, response.c_str(), response.size(), 0) < 0) {
            break;
        }
    }
}
string ControlServer::process_command(string command) {
    ostringstream response;
    if(command=="ping") {
        response<<"pong"<<endl;
        return response.str();
    }
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
            response<<"INVALID COMMAND!!"<<endl;
            return response.str();
        }
        if(command_tokens.size()>2 && command_tokens[1]=="drain") {
            pool.drain_backend(command_tokens[2],stoi(command_tokens[3]));
            pool.list_backend();
            return response.str();
        }else if(command_tokens.size()>1 && command_tokens[1]=="info") {
            Config config=ConfigLoader::load(toml_file);
            vector<double>backend_percentiles=registry.backend_connection_latency.estimate_percentiles();
            vector<double>client_session_percentiles=registry.client_duration.estimate_percentiles();

            response<<endl;
            response<<"----------------------------------------------------------------------"<<endl;
            response<<"[ CONNECTION METRICS ]"<<endl;
            response<<"TOTAL CONNECTIONS: "<<registry.get_total_connections()<<endl;
            response<<"ACTIVE CONNECTIONS: "<<registry.get_active_connections()<<endl;
            response<<"TOTAL ERRORS: "<<registry.get_total_errors()<<endl;
            response<<"ALGORITHM: "<<config.algorithm<<endl;
            response<<endl;

            response<<"----------------------------------------------------------------------"<<endl;
            response<<"[ LATENCY METRICS]"<<endl;
            response<<"Client Session Duration:"<<endl;
            response<<"P50: "<<"     "<<client_session_percentiles[0]<<" ms"<<endl;
            response<<"P95: "<<"     "<<client_session_percentiles[1]<<" ms"<<endl;
            response<<"P99: "<<"     "<<client_session_percentiles[2]<<" ms"<<endl;
            response<<endl;

            response<<"Backend Connection Latency:"<<endl;
            response<<"P50: "<<"     "<<backend_percentiles[0]<<" ms"<<endl;
            response<<"P95: "<<"     "<<backend_percentiles[1]<<" ms"<<endl;
            response<<"P99: "<<"     "<<backend_percentiles[2]<<" ms"<<endl;
            response<<endl;

            response<<"----------------------------------------------------------------------"<<endl;
            response<<"[ BACKEND POOL ]"<<endl;
            response<<pool.list_backend();
            return response.str();
        }else if(command_tokens.size()>1 && command_tokens[1]=="reload") {
            Config new_config=ConfigLoader::load(toml_file);

            pool.update_backends(new_config.backends);
            response<<"NEW BACKENDS WERE KEPT INTO IMPLEMENTATION!!"<<endl;
            monitor.update_drainout_time(new_config.drainout_time);
            response<<"DRAINOUT TIME UPDATED!"<<endl;

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
                response<<"ALGORITHM RELOADED SUCCESSFULLY"<<endl;
            }else{
                response<<"INVALID ALGORITHM"<<endl;
            }
            return response.str();
        }else{
            response<<"INVALID COMMAND!!"<<endl;
            return response.str();
        }

    return response.str();
}