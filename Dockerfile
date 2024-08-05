FROM alpine:3.20.2 AS builder

RUN apk update && apk add --no-cache cmake g++ make

WORKDIR /app

COPY . .

RUN mkdir build && cd build && cmake -DCMAKE_BUILD_TYPE=Release .. && make

FROM alpine:3.20.2

WORKDIR /app/build

COPY --from=builder /app/build/server /app/build/server

COPY --from=builder /app/assets /app/assets

RUN chmod +x /app/build/server

CMD ["./server"]