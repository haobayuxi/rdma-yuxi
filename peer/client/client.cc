#include "client.h"

#include <assert.h>
#include <stdio.h>

#include <chrono>
#include <ctime>

#include "rlib/rdma_ctrl.hpp"

int client_node_id = 0;
int tcp_port = 10001;
int server_port = 10001;

using namespace rdmaio;
using namespace std::chrono;

int main(int argc, char *argv[]) {
  int tcp_port = 10001;

  auto rdma_ib_info = (context_info *)malloc(sizeof(context_info));
  open_device_and_alloc_pd(rdma_ib_info);

  auto fd = dialTo("192.168.3.72", tcp_port);

  rdma_fd *handler = (rdma_fd *)malloc(sizeof(rdma_fd));
  handler->fd = fd;
  get_context_info(handler, rdma_ib_info);
  build_rdma_connection(handler);

  printf("init done\n");
  char *a = "hello world";
  rdma_write(handler, a, 10);
  return 0;
}
