# Dockerfile for Building and Running Astron

# Use the latest Ubuntu as the base image
FROM ubuntu:latest

# Install necessary dependencies to build Astron
RUN apt-get update && \
    apt-get install -y \
    cmake \
    libyaml-cpp-dev \
    libmongoclient-dev \
    llvm \
    libboost-dev \
    clang \
    g++ \
    gcc \
    libuv1-dev

# Copy the local source directory into the container and set the working directory
COPY . /app
WORKDIR /app

# Build Astron and set the application entrypoint
RUN mkdir -p build

RUN cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    make

# Set the executable as the entrypoint
ENTRYPOINT [ "./build/astrond" ]