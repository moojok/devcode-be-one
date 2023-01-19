#include <stdio.h>
#include <string.h>
#include <microhttpd.h>

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
    if (strcmp(url, "/hello") != 0) {
        return MHD_NO;
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

    daemon = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION,
                              PORT,
                              NULL,
                              NULL,
                              &handle_access,
                              NULL,
                              MHD_OPTION_END);
    if (NULL == daemon) {
        return 1;
    }

    getchar();

    MHD_stop_daemon(daemon);

    return 0;
}
