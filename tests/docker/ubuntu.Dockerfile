FROM ubuntu:22.04

# Install dependencies
RUN apt-get update
RUN apt-get install -y \
        python3.11 \
        python3-pip \
        pkg-config \
        ccache \
        clang \
        g++ \
        ninja-build \
        cmake \
        libjsoncpp-dev \
        libexpected-dev \
        libgtest-dev \
        libfmt-dev \
        libcxxopts-dev
RUN apt-get clean
RUN update-alternatives --install /usr/local/bin/python python /usr/bin/python3.11 10
# Install meson from pip
RUN python3 -m pip install -U meson==0.64.1
