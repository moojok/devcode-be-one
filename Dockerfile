FROM alpine:latest
RUN apk add --no-cache gcc musl-dev libmicrohttpd-dev
COPY main.c /
RUN gcc -o server main.c -lmicrohttpd
CMD ["./server"]
EXPOSE 3030
