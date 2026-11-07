
FROM ubuntu:24.04

WORKDIR /app

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    && rm -rf /var/lib/apt/lists/*

# Copy project source
COPY . .

# Configure and compile
RUN cmake -S . -B build && \
    cmake --build build -j$(nproc)

# Load balancer listening port
EXPOSE 2424

# Start the load balancer
CMD ["./build/load_balancer"]
