#include "client.h"

#include <assert.h>
#include <stdio.h>

#include <chrono>
#include <ctime>

int client_node_id = 0;
int tcp_port = 10001;
int server_port = 10001;

char *gen_test_string(int len, int times) {
  char *str;
  str = (char *)malloc(len + 1);

  sprintf(str, "send msg %d times", times);
  str[len] = '1';

  return str;
}

int main(int argc, char *argv[]) {
  rdma_fd *handler = (rdma_fd *)malloc(sizeof(rdma_fd));

  init_client(handler, "192.168.3.72");
  srand((unsigned)time(NULL));
  uint32_t buf_size = 20;
  for (int i = 0; i < 2; i++) {
    char *buf = gen_test_string(buf_size, i);
    printf("buf = %s\n", buf);
    client_send(handler, buf, buf_size);
    free(buf);
    char *response = client_recv(handler);
    printf("get response = %s\n", response);
    free(response);
  }

  return 0;
}
