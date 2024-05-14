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

# Get Python for LIT
RUN add-apt-repository ppa:deadsnakes/ppa
RUN apt install -y python3.10 python3-pip
RUN python3 -m pip install lit

RUN apt install -y ninja-build
RUN apt install -y jq

RUN apt install -y lsb-release
RUN wget https://apt.llvm.org/llvm.sh
RUN chmod +x llvm.sh
RUN ./llvm.sh 17 all
RUN apt install -y llvm-dev

# Install dependencies
RUN apt install -y libcurl4-gnutls-dev
RUN apt install -y libzstd-dev
RUN apt install -y libedit-dev

# Cleanup
RUN rm llvm.sh

# Copy maki files into docker
COPY . .
