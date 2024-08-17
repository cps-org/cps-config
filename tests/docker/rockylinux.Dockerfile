FROM rockylinux:9

# Enable EPEL
# Install dependencies
RUN dnf update -y && \
    dnf install -y 'dnf-command(config-manager)' && \
    dnf config-manager --set-enabled crb -y && \
    dnf install epel-release -y && \
    dnf install -y \
        python3.11 \
        python3-pip \
        pkgconf-pkg-config \
        ccache \
        clang \
        g++ \
        ninja-build \
        cmake \
        json-devel \
        expected-devel \
        gtest-devel \
        fmt-devel \
        cli11-devel && \
    dnf clean all
RUN update-alternatives --install /usr/local/bin/python python /usr/bin/python3.11 10
# Install meson from pip
RUN python3 -m pip install -U meson==0.64.1
