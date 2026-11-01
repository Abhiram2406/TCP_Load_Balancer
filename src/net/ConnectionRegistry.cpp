#include "ConnectionRegistry.hpp"
#include "../common.hpp"
void ConnectionRegistry::register_connection(int client_fd,int backend_fd) {
    lock_guard<mutex>lock(connections_mutex);
    ConnectionInfo conn(client_fd,backend_fd);
    connections.push_back(conn);
}
void ConnectionRegistry::unregister_connection(int c_fd,int b_fd) {
    lock_guard<mutex>lock(connections_mutex);
    for (auto it = connections.begin(); it != connections.end(); ++it) {
        if (it->client_fd == c_fd && it->backend_fd == b_fd) {
            connections.erase(it);
            return;
        }
    }
}
void ConnectionRegistry::shutdown_all_connections() {
    vector<ConnectionInfo> active_connections;
    {
        lock_guard<mutex> lock(connections_mutex);
        active_connections = connections;
        connections.clear();
    }
    for (const auto& conn : active_connections) {
        if (conn.client_fd >= 0) {
            shutdown(conn.client_fd, SHUT_RDWR);
        }

        if (conn.backend_fd >= 0) {
            shutdown(conn.backend_fd, SHUT_RDWR);
        }
    }
}
size_t ConnectionRegistry::get_active_count() {
    lock_guard<mutex>lock(connections_mutex);
    return connections.size();
}