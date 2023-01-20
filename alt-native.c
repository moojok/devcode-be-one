#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>

#define NUM_CONNECTIONS 1000
#define NUM_THREADS 8
#define PORT 3030

struct timeval previous_time;
double rps;
pthread_mutex_t rps_mutex;
pthread_t rps_thread;

int my_socket;

int handle_request(int client_socket) {
    char buffer[1024] = {0};
    int valread = read(client_socket, buffer, 1024);
    char *response = "{\"message\": \"Hello world\"}";

    // Parse the request to check if the url is "/hello"
    if (strstr(buffer, "GET /hello") != NULL) {
        char *http_response = malloc(strlen(response) + strlen("HTTP/1.1 200 OK\nContent-Type: application/json\nContent-Length: ") + 16);
        sprintf(http_response, "HTTP/1.1 200 OK\nContent-Type: application/json\nContent-Length: %d\n\n%s", (int)strlen(response), response);
        send(client_socket, http_response, strlen(http_response), 0);
        free(http_response);
        return 1;
    }
    return 0;
}

void* handle_connections(void* arg) {
    int client_socket;
    struct sockaddr_in client_address;
    socklen_t client_len = sizeof(client_address);

    while (1) {
        client_socket = accept(my_socket, (struct sockaddr *)&client_address, &client_len);

        if (handle_request(client_socket)) {
            pthread_mutex_lock(&rps_mutex);
            rps++;
            pthread_mutex_unlock(&rps_mutex);
        }

        close(client_socket);
    }
    return NULL;
}

void* calculate_rps(void* arg) {
    while(1) {
        sleep(1);
        pthread_mutex_lock(&rps_mutex);
        printf("RPS: %.2f\n", rps);
        rps = 0;
        pthread_mutex_unlock(&rps_mutex);
    }
    return NULL;
}

void handle_sigint(int sig) {
    close(my_socket);
    exit(0);
}

int main(int argc, char *argv[]) {
    signal(SIGINT, handle_sigint);
    pthread_mutex_init(&rps_mutex, NULL);

    struct sockaddr_in server_address;
    my_socket = socket(AF_INET, SOCK_STREAM, 0);
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    bind(my_socket, (struct sockaddr *)&server_address, sizeof(server_address));
    listen(my_socket, NUM_CONNECTIONS);

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_t thread;
        pthread_create(&thread, NULL, handle_connections, NULL);
    }

    pthread_create(&rps_thread, NULL, calculate_rps, NULL);

    while(1) {
        sleep(1);
    }

    close(my_socket);

    return 0;
}
