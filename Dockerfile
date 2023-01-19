FROM alpine:latest

RUN apk add --update \
    build-base \
    libmicrohttpd-dev

COPY . /usr/src/app

WORKDIR /usr/src/app

RUN gcc -o server main.c -lmicrohttpd && \
    rm main.c

EXPOSE 3030
CMD ["./server"]
