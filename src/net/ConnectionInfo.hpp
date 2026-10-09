struct ConnectionInfo {
    int client_fd;
    int backend_fd;
    ConnectionInfo(int a,int b):client_fd(a),backend_fd(b) {};
};