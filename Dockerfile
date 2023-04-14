# Use an official Ubuntu base image
FROM ubuntu:22.04

# Set environment variables
ENV DEBIAN_FRONTEND=noninteractive
ENV TZ=UTC

# Set the working directory
WORKDIR /app

# Update package lists
RUN apt-get update

# Install build dependencies
RUN apt-get install -y \
        build-essential \
        wget \
        git

# Install SDL2 dependencies
RUN apt-get install -y \
        libsdl2-dev \
        libsdl2-image-dev \
        libsdl2-mixer-dev \
        libsdl2-ttf-dev

# Install Boost.System and ncurses
RUN apt-get install -y \
        libboost-system-dev \
        libncurses-dev

# Clean up package lists
RUN apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# Install CMake 3.26
RUN wget -qO- "https://cmake.org/files/v3.26/cmake-3.26.0-linux-x86_64.tar.gz" | tar --strip-components=1 -xz -C /usr/local

# Set working directory for ctest
WORKDIR /app/build/docker

# Run tests with ctest
CMD ["ctest"]
