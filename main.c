#include <stdio.h>
#include <string.h>
#include <microhttpd.h>
#include <sys/time.h>

#define PORT 3030
#define IP "0.0.0.0"

int handle_access(void *cls,
                  struct MHD_Connection *connection,
                  const char *url,
                  const char *method,
                  const char *version,
                  const char *upload_data,
                  size_t *upload_data_size,
                  void **ptr)
{
    static struct timeval previous_time;
    struct timeval current_time;
    double rps;

    if (strcmp(url, "/hello") != 0) {
        return MHD_NO;
    }

    gettimeofday(&current_time, NULL);
    if (previous_time.tv_sec == 0 && previous_time.tv_usec == 0) {
        previous_time = current_time;
    } else {
        double elapsed_time = (current_time.tv_sec - previous_time.tv_sec) * 1000.0;
        elapsed_time += (current_time.tv_usec - previous_time.tv_usec) / 1000.0;
        rps = 1000.0 / elapsed_time;
        printf("RPS: %.2f\n", rps);
        previous_time = current_time;
    }

    struct MHD_Response *response;
    char *json = "{\"message\": \"Hello world\"}";

    response = MHD_create_response_from_buffer(strlen(json),
                                               json,
                                               MHD_RESPMEM_MUST_COPY);
    MHD_add_response_header(response, "Content-Type", "application/json");
    MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    return MHD_YES;
}

int main(int argc, char *argv[])
{
    struct MHD_Daemon *daemon;
    unsigned int flags = MHD_USE_SELECT_INTERNALLY;

    daemon = MHD_start_daemon(flags,
                              PORT,
                              NULL,
                              NULL,
                              &handle_access,
                              NULL,
                              MHD_OPTION_CONNECTION_LIMIT, 1000,
                              MHD_OPTION_THREAD_POOL_SIZE, 8,
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
