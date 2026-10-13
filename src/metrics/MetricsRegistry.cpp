#include "MetricsRegistry.hpp"
void MetricsRegistry::record_connection() {
    total_connections.fetch_add(1);
}
void MetricsRegistry::update_connection(bool done) {
    if(done) {
        active_connections.fetch_sub(1);
    }else{
        active_connections.fetch_add(1);
    }
}
void MetricsRegistry::record_error() {
    total_errors.fetch_add(1);
}
uint64_t MetricsRegistry::get_total_connections() {
    return total_connections.load();
}
int MetricsRegistry::get_active_connections() {
    return active_connections.load();
}
uint64_t MetricsRegistry::get_total_errors() {
    return total_errors.load();
}