FROM frolvlad/alpine-gcc
RUN apk add --no-cache libmicrohttpd-dev
WORKDIR /app
COPY main.c /app/main.c
RUN gcc -o server main.c -lmicrohttpd
CMD [ "/app/server" ]
