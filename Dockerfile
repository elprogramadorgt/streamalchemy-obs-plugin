FROM ubuntu:22.04

ARG USER_UID=1000
ENV CMAKE_VERSION=4.0.3
ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
	build-essential \
	ca-certificates \
	gnupg \
	libgles2-mesa-dev \
	libqt6svg6-dev \
	ninja-build \
	pkg-config \
	qt6-base-dev \
	qt6-base-private-dev \
	software-properties-common \
	wget

RUN add-apt-repository -y ppa:obsproject/obs-studio \
	&& apt-get update \
	&& apt-get install -y --no-install-recommends libobs-dev

# Install  Cmake 4.0.3
RUN wget -q https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-linux-x86_64.sh \
	&& mkdir -p /opt/cmake \
	&& bash cmake-${CMAKE_VERSION}-linux-x86_64.sh --skip-license --prefix=/opt/cmake \
	&& ln -sf /opt/cmake/bin/cmake /usr/local/bin/cmake \
	&& rm cmake-${CMAKE_VERSION}-linux-x86_64.sh

WORKDIR /data
USER ${USER_UID}

