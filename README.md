# C++ Layer 4 Load Balancer
[![Docker Hub](https://img.shields.io/docker/v/24abhiram/cpp-load-balancer?label=Docker%20Hub)](https://hub.docker.com/r/24abhiram/cpp-load-balancer)

A configurable Layer 4 (TCP) load balancer built in C++ using Linux/POSIX sockets. It accepts client TCP connections, selects a backend using a configurable algorithm, and forwards data between the client and the selected backend.

## Features

### TCP Proxying
- Accepts incoming TCP connections.
- Establishes a connection to the selected backend.
- Proxies data between the client and backend.

### Load-Balancing Algorithms
- **Round Robin:** Selects backends in cyclic order.
- **Weighted Round Robin:** Distributes connections according to configured backend weights.
- **Random:** Randomly selects a healthy backend.
- **Least Connections:** Selects a backend based on its active connection count.
- **IP Hash:** Selects a backend using the client's IP address.

### Backend and Health Management
- Configurable backend pool.
- Active and passive health checks.
- Backend health-state management.
- Graceful backend draining and undraining.
- Runtime backend addition and removal.

### Runtime Configuration and Control
- TOML-based configuration.
- Runtime configuration reload.
- Runtime load-balancing algorithm switching.
- Unix domain socket-based control server.
- Separate command-line client (`lb-cli`).

### Metrics
- Total accepted client connections.
- Active connections.
- Error count.
- Backend connection latency histogram.
- Client session duration histogram.
- P50, P95 and P99 latency estimates.

## Architecture

| Module | Responsibility |
|---|---|
| `net/` | TCP listener and TCP connection handling |
| `pool/` | Backend representation and backend-pool management |
| `balancer/` | Load-balancing algorithm implementations |
| `health/` | Active and passive backend health monitoring |
| `config/` | TOML configuration loading |
| `metrics/` | Connection counters and latency measurements |
| `control/` | Runtime control server and command processing |
| `tests/` | TCP client test utilities |

### High-Level Data Flow

```text
                         TCP CLIENTS
                              |
                              v
                    +-------------------+
                    |   TCP LISTENER    |
                    +-------------------+
                              |
                              v
                    +-------------------+
                    | LOAD BALANCER     |
                    | Algorithm         |
                    | Backend Pool      |
                    +-------------------+
                       |       |       |
                       v       v       v
                    Backend  Backend  Backend
                       1       2       3

                       CONTROL PLANE
                              |
                            lb-cli
                              |
                     Unix Domain Socket
                              |
                       Control Server
```

## Configuration

The load balancer reads its configuration from `config/lb.toml`.

Example:

```toml
listen_port = 2424
algorithm = "Weighted Round Robin"
drainout_time = 20

[[backends]]
host = "127.0.0.1"
port = 9001
weight = 1

[[backends]]
host = "127.0.0.1"
port = 9002
weight = 2

[[backends]]
host = "127.0.0.1"
port = 9003
weight = 3
```

- `listen_port`: TCP port on which the load balancer accepts clients.
- `algorithm`: Algorithm selected at startup.
- `drainout_time`: Time allowed for connections to finish during backend draining.
- `host` and `port`: Backend address.
- `weight`: Backend weight used by Weighted Round Robin.

## Build and Run

### Prerequisites
- Linux environment (Ubuntu or WSL2 with Ubuntu).
- C++ compiler with C++17 support.
- CMake.
- Python 3 for the TCP client test utility.

## Docker Image

The load balancer is available as a Docker image on Docker Hub.

**Docker Hub:** [24abhiram/cpp-load-balancer](https://hub.docker.com/r/24abhiram/cpp-load-balancer)

### Pull the Image

```bash
docker pull 24abhiram/cpp-load-balancer:v1
```

### Run the Container

```bash
docker run -d \
  --name my-load-balancer \
  -p 2424:2424 \
  24abhiram/cpp-load-balancer:v1
```

### Check Container Status

```bash
docker ps
```

### View Container Logs

```bash
docker logs my-load-balancer
```

The container exposes TCP port `2424` for incoming client connections.

Backend servers must be reachable from the container, and the backend addresses should be configured accordingly.

### Build Commands

Run these commands from the repository root:

```bash
cmake -S . -B build
cmake --build build
```

Start the load balancer:

```bash
./build/load_balancer
```

Keep the load balancer running in this terminal.

## Running the TCP Client

Open another terminal from the repository root and run:

```bash
python3 tests/tcp_client.py
```

The client connects to the load balancer, sends a message and prints the response returned by the selected backend. The stress-test client can be configured by changing the client and worker counts in the script.

## Runtime Control

The control server exposes commands through a local Unix domain socket. The `lb-cli` program communicates with this control server.

To use runtime-control commands, open a **separate terminal** from the repository root and start the CLI:

```bash
./build/lb-cli
```

After the CLI starts, enter the supported commands in its prompt. Keep the load balancer running in the other terminal.

Commands implemented in the project include:

| Command | Purpose |
|---|---|
| `bob info` | Displays algorithm, connection metrics, latency metrics and backend information |
| `bob stats` | Displays connection and latency metrics |
| `bob status` | Displays backend-pool status |
| `bob drain <host> <port>` | Stops sending new connections to a backend while existing connections finish |
| `bob undrain <host> <port>` | Makes a drained backend eligible for new connections again |
| `bob add <host> <port> <weight>` | Adds a backend |
| `bob remove <host> <port>` | Removes a backend |
| `bob set <algorithm>` | Changes the active algorithm |
| `bob reload` | Reloads the TOML configuration |

Use the command spelling and argument format expected by the `lb-cli` implementation.

## Testing

Testing has used local TCP backends and a custom Python TCP client. Scenarios include backend distribution, Weighted Round Robin verification, backend health behavior, runtime algorithm switching, configuration reload, concurrent client connections, thread cleanup and process-resource observation.

See `TESTING_AND_NOTES.txt` for recorded test results and measurements.