FROM ubuntu:22.04

WORKDIR /maki

## Set up the base Ubuntu image
RUN apt -y update
RUN apt -y upgrade
RUN apt install -y build-essential
RUN apt-get update -qq -y
RUN apt-get install -qq -y wget

RUN apt install -y software-properties-common

RUN apt install -y git

# Get latest cmake
RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /etc/apt/trusted.gpg.d/kitware.gpg >/dev/null
RUN apt-add-repository 'deb https://apt.kitware.com/ubuntu/ jammy main'
RUN apt update
RUN apt install -y cmake


RUN apt install -y ninja-build
RUN apt install -y jq
RUN add-apt-repository ppa:deadsnakes/ppa
RUN apt install -y python3.10 python3-pip
RUN python3 -m pip install -U numpy
RUN python3 -m pip install -U scan-build
RUN python3 -m pip install lit

RUN apt install -y lsb-release
RUN wget https://apt.llvm.org/llvm.sh
RUN chmod +x llvm.sh
RUN ./llvm.sh 17 all
RUN apt install -y llvm-dev

# Install dependencies for evaluation programs
RUN apt install -y libcurl4-gnutls-dev
RUN apt install -y libzstd-dev
RUN apt install -y libedit-dev
RUN apt install -y autoconf
RUN apt install -y ed
RUN apt install -y gnutls-bin
RUN apt install -y help2man
RUN apt install -y libgif-dev
RUN apt install -y libjpeg-dev
RUN apt install -y libmotif-dev
RUN apt install -y libpng-dev
RUN apt install -y libtiff-dev
RUN apt install -y libx11-dev
RUN apt install -y libxmu-dev
RUN apt install -y libxmu-headers
RUN apt install -y libxpm-dev
RUN apt install -y texinfo
RUN apt install -y unzip
RUN apt install -y xaw3dg-dev

# Cleanup
RUN rm llvm.sh
# RUN git clone https://github.com/appleseedlab/maki.git

# Copy maki files into docker
COPY . .
