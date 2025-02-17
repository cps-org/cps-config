FROM ubuntu:22.04

# Install dependencies
RUN apt-get update && \
    apt-get install -y \
        python3.11 \
        python3-pip \
        pkg-config \
        ccache \
        clang \
        g++ \
        ninja-build \
        cmake \
        nlohmann-json3-dev \
        libexpected-dev \
        libgtest-dev \
        libfmt-dev \
        libcli11-dev \
        flex \
        bison && \
    apt-get clean
RUN update-alternatives --install /usr/local/bin/python python /usr/bin/python3.11 10
# Install meson from pip
RUN python3 -m pip install -U meson==0.64.1
