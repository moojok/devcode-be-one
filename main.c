#include <microhttpd.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define NUM_CONNECTIONS 1000
#define NUM_THREADS 8
#define PORT 3030

struct timeval previous_time;
double rps;
pthread_mutex_t rps_mutex;
pthread_t rps_thread;

struct MHD_Daemon *my_daemon;

int handle_access(void *cls, struct MHD_Connection *connection, const char *url,
                  const char *method, const char *version,
                  const char *upload_data, size_t *upload_data_size,
                  void **ptr) {
  const char json[] = "{\"message\": \"Hello world\"}";
  char *json_copy = strdup(json);

  struct MHD_Response *response;

  if (strcmp(url, "/hello") == 0) {
    response = MHD_create_response_from_buffer(strlen(json_copy), json_copy,
                                               MHD_RESPMEM_MUST_FREE);
    MHD_add_response_header(response, "Content-Type", "application/json");
    MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return MHD_YES;
  }
  return MHD_NO;
}

void *calculate_rps(void *arg) {
  while (1) {
    sleep(1);
    pthread_mutex_lock(&rps_mutex);
    printf("RPS: %.2f\n", rps);
    pthread_mutex_unlock(&rps_mutex);
  }
  return NULL;
}

void handle_sigint(int sig) {
  MHD_stop_daemon(my_daemon);
  exit(0);
}

int main(int argc, char *argv[]) {
  signal(SIGINT, handle_sigint);
  pthread_mutex_init(&rps_mutex, NULL);

  my_daemon = MHD_start_daemon(
      MHD_USE_SELECT_INTERNALLY, PORT, NULL, NULL, &handle_access, NULL,
      MHD_OPTION_CONNECTION_LIMIT, NUM_CONNECTIONS, MHD_OPTION_THREAD_POOL_SIZE,
      NUM_THREADS, MHD_OPTION_END);
  if (NULL == my_daemon) {
    return 1;
  }

  pthread_create(&rps_thread, NULL, calculate_rps, NULL);

  while (1) {
    sleep(1);
  }

  MHD_stop_daemon(my_daemon);

  return 0;
}
