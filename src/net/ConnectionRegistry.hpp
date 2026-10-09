#pragma once
#include<mutex>
#include<vector>
#include "ConnectionInfo.hpp"
using namespace std;

class ConnectionRegistry {
    vector<ConnectionInfo>connections;
    mutex connections_mutex;
    public:
    void register_connection(int client_fd,int backend_fd);
    void unregister_connection(int client_fd,int backend_fd);
    void shutdown_all_connections();
    size_t get_active_count();
};