FROM frolvlad/alpine-gcc
RUN apk add --no-cache pcre-dev libc-dev
WORKDIR /app
COPY alt-fast.c /app/main.c
RUN gcc -o server main.c -lpcre -lpthread
CMD ["/app/server"]
