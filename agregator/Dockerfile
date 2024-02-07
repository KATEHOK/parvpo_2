# Use the official debian image as the base image
FROM debian:12

# Set the working directory inside the container
WORKDIR /app

RUN echo "deb http://mirror.yandex.ru/debian/ bookworm main" > /etc/apt/sources.list

# Install Boost dependencies
RUN apt-get update --fix-missing && apt-get upgrade
RUN apt-get install -y build-essential
RUN apt-get install -y libboost-all-dev
RUN apt-get install -y libssl-dev
RUN apt-get install -y git
RUN apt-get install -y cmake
RUN apt-get install libcurl4-openssl-dev

RUN git clone https://github.com/whoshuu/cpr.git && \
    cd cpr && \
    cmake . && \
    make && \
    make install && \
    cd /app

COPY agregator.cpp .
COPY ./crow_all.h .
COPY ./json.hpp .

RUN g++ agregator.cpp -o agregator -lcpr -lpthread

CMD export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH; ./agregator

