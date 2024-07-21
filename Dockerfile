FROM ubuntu:24.04 AS builder

RUN apt update && apt install -y cmake g++ make

WORKDIR /app

COPY . .

RUN mkdir build && cd build && cmake .. && make

RUN chmod +x /app/build/server

WORKDIR /app/build

CMD ["./server"]