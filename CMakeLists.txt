cmake_minimum_required(VERSION 3.16)

project(LoadBalancer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    tomlplusplus
    GIT_REPOSITORY https://github.com/marzer/tomlplusplus.git
    GIT_TAG v3.4.0
)

FetchContent_MakeAvailable(tomlplusplus)

add_executable(load_balancer
    src/main.cpp
    src/net/TcpConnection.cpp
    src/net/TcpListener.cpp
    src/net/ConnectionRegistry.cpp
    src/pool/Backend.cpp
    src/pool/BackendPool.cpp
    src/balancer/RoundRobin.cpp
    src/balancer/WeightedRR.cpp
    src/balancer/Random.cpp
    src/balancer/LeastConnections.cpp
    src/balancer/IpHash.cpp
    src/config/ConfigLoader.cpp
    src/health/HealthStateMachine.cpp
    src/health/HealthChecker.cpp
    src/balancer/DrainMonitor.cpp
    src/metrics/MetricsRegistry.cpp
    src/metrics/Histogram.cpp
    src/control/ControlServer.cpp
)

target_link_libraries(load_balancer PRIVATE tomlplusplus::tomlplusplus)