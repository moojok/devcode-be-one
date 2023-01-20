#include <stdio.h>
#include <string.h>
#include <microhttpd.h>
#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

#define PORT 3030
#define IP "0.0.0.0"
#define NUM_THREADS 8
#define NUM_CONNECTIONS 1000

static char json[] = "{\"message\": \"Hello world\"}";
static int rps_count = 0;
static pthread_mutex_t rps_mutex = PTHREAD_MUTEX_INITIALIZER;

int handle_access(void *cls,
                  struct MHD_Connection *connection,
                  const char *url,
                  const char *method,
                  const char *version,
                  const char *upload_data,
                  size_t *upload_data_size,
                  void **ptr)
{
    struct MHD_Response *response;

    if (strcmp(url, "/hello") != 0) {
        return MHD_NO;
    }

    pthread_mutex_lock(&rps_mutex);
    rps_count++;
    pthread_mutex_unlock(&rps_mutex);

    response = MHD_create_response_from_buffer(strlen(json),
                                               json,
                                               MHD_RESPMEM_PERSISTENT);
    MHD_add_response_header(response, "Content-Type", "application/json");
    MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    return MHD_YES;
}

void *calculate_rps(void *arg)
{
    int previous_count = 0;
    struct timeval previous_time;
    struct timeval current_time;

    gettimeofday(&previous_time, NULL);

    while(1) {
        sleep(1);

        gettimeofday(&current_time, NULL);
        double elapsed_time = (current_time.tv_sec - previous_time.tv_sec) * 1000.0;
        elapsed_time += (current_time.tv_usec - previous_time.tv_usec) / 1000.0;

        pthread_mutex_lock(&rps_mutex);
        int current_count = rps_count;
        rps_count = 0;
        pthread_mutex_unlock(&rps_mutex);

        double rps = (current_count - previous_count) * 1000.0 / elapsed_time;
        printf("RPS: %.2f\n", rps);

        previous_count = current_count;
        previous_time = current_time;
    }
}

int main(int argc, char *argv[])
{
    struct MHD_Daemon *daemon;
    unsigned int flags = MHD_USE_SELECT_INTERNALLY;
    pthread_t rps_thread;

        pthread_create(&rps_thread, NULL, calculate_rps, NULL);

    daemon = MHD_start_daemon(flags,
                              PORT,
                              NULL,
                              NULL,
                              &handle_access,
                              NULL,
                              MHD_OPTION_CONNECTION_LIMIT, NUM_CONNECTIONS,
                              MHD_OPTION_THREAD_POOL_SIZE, NUM_THREADS,
                              MHD_OPTION_END);
    if (NULL == daemon) {
        return 1;
    }

    while(1) {
        sleep(1);
    }

    MHD_stop_daemon(daemon);

    return 0;
}
