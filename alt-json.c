#include <stdio.h>
#include <string.h>
#include <microhttpd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define PORT 3030
#define IP "0.0.0.0"

char *map;
size_t map_size;

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

    if (strcmp(url, "/hello") == 0) {
        response = MHD_create_response_from_buffer(map_size, map, MHD_RESPMEM_MUST_COPY);
        MHD_add_response_header(response, "Content-Type", "application/json");
        MHD_queue_response(connection, MHD_HTTP_OK, response);
        MHD_destroy_response(response);
        return MHD_YES;
    }
    return MHD_NO;
}

int main(int argc, char *argv[])
{
    struct MHD_Daemon *daemon;
    unsigned int flags = MHD_USE_SELECT_INTERNALLY;
    int fd;

    fd = open("response.json", O_RDONLY);
    map_size = lseek(fd, 0, SEEK_END);
    map = mmap(NULL, map_size, PROT_READ, MAP_PRIVATE, fd, 0);

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

    munmap(map, map_size);
    close(fd);
    MHD_stop_daemon(daemon);

    return 0;
}
