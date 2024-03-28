FROM ubuntu:22.04

LABEL maintainer="pappasbrent@gmail.com"

# Create the directoy /maki in the image, and make it the working directory
WORKDIR /maki

## Set up the base Ubuntu image
RUN apt -y update
RUN apt -y upgrade
RUN apt install -y build-essential

# Install dependencies for Maki's Clang frontend
RUN apt install -y clang-14
RUN apt install -y cmake
RUN apt install -y libclang-14-dev
RUN apt install -y llvm-14

# Install dependencies for Maki's Python scripts
RUN apt install -y software-properties-common
RUN add-apt-repository ppa:deadsnakes/ppa
RUN apt install -y python3.10 python3-pip
RUN python3 -m pip install -U numpy
RUN python3 -m pip install -U scan-build

# Install dependencies for evaluation programs
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
RUN apt install -y wget
RUN apt install -y xaw3dg-dev

# Copy all folders in the current directory into the Docker image, except for
# those listed in .dockerignore
COPY . .
