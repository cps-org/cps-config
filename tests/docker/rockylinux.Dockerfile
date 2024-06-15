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
        pkgconf-pkg-config \
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
RUN python3 -m pip install -U meson==0.64.1
