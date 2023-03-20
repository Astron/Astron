FROM ubuntu:latest             

# Install our required dependencies to build Astron
RUN set -ex;                                                                                            \
    apt-get update;                                                                                     \
    apt-get install -y cmake libyaml-cpp-dev libmongoclient-dev llvm libboost-dev clang g++ gcc libuv1-dev;              

# Copy the source directory into the container and set our new working directory
COPY . /app
WORKDIR /app

# Build Astron and set our application entrypoint
RUN cd build && cmake .. && make
ENTRYPOINT [ "./build/astrond" ]