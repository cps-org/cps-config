FROM rockylinux:9

# Enable EPEL
RUN dnf update -y
RUN dnf install -y 'dnf-command(config-manager)'
RUN dnf config-manager --set-enabled crb -y
RUN dnf install epel-release -y

# Install dependencies
RUN dnf install -y \
        python3.11 \
        python3-pip \
        ccache \
        clang \
        g++ \
        ninja-build \
        cmake \
        jsoncpp-devel \
        expected-devel \
        gtest-devel \
        fmt-devel \
        cxxopts-devel
RUN dnf clean all
RUN update-alternatives --install /usr/local/bin/python python /usr/bin/python3.11 10
# Install meson from pip
RUN python3 -m pip install -U meson

# Copy code
WORKDIR /workarea
COPY ./ ./

ARG cc=gcc
ARG cxx=g++
ARG setup_options=

# Build cps-config and tests
ENV CC="ccache $cc" CXX="ccache $cxx"
ENV CCACHE_DIR=/ccache
RUN meson setup builddir $setup_options
RUN --mount=type=cache,target=/ccache/ ninja -C builddir
