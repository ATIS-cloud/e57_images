FROM ubuntu:22.04
LABEL authors="Mehdi"

WORKDIR /app

RUN apt update  \
    && apt-get install build-essential -y  \
    && apt-get install cmake -y  \
    && apt-get install wget -y \
    && apt-get install libicu-dev -y \
    && apt-get install nlohmann-json3-dev -y

RUN wget https://boostorg.jfrog.io/artifactory/main/release/1.83.0/source/boost_1_83_0.tar.bz2
RUN tar --bzip2 -xf boost_1_83_0.tar.bz2  \
    && cd boost_1_83_0  \
    && ./bootstrap.sh --with-libraries=program_options,system,thread,filesystem  \
    && ./b2 -j$(nproc) --prefix=/opt/boost install

RUN wget https://archive.apache.org/dist/xerces/c/3/sources/xerces-c-3.1.0.tar.gz
RUN tar -xf xerces-c-3.1.0.tar.gz && cd xerces-c-3.1.0 && ./configure --prefix=/opt/xerces && make && make install
