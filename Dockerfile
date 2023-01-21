FROM frolvlad/alpine-gcc
RUN apk add --no-cache libc-dev
WORKDIR /app
COPY alt-fast.c /app/main.c
RUN gcc -o server main.c -lpthread
CMD [ "/app/server" ]
