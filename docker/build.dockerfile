FROM alpine:3.20.2 AS builder

RUN apk update && apk add --no-cache g++ make cmake

WORKDIR /app

COPY example /app/example
COPY include /app/include
COPY lib /app/lib
COPY src /app/src
COPY CMakeLists.txt /app/CMakeLists.txt

RUN mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_EXE_LINKER_FLAGS="-static" .. && make

RUN chmod +x /app/build/server

FROM scratch

WORKDIR /app/build

COPY --from=builder /app/build/server /app/build/server

COPY assets /app/assets

ENTRYPOINT ["/app/build/server"]
