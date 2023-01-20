FROM frolvlad/alpine-gcc
RUN apk add --no-cache libmicrohttpd-dev libc-dev
WORKDIR /app
COPY main.c /app/main.c
RUN gcc -o server main.c -lmicrohttpd -lpthread
CMD [ "/app/server" ]
