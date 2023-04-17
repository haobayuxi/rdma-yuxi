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
  auto handler = std::make_shared<Handler>();
  handler->set_fd(fd);
  handler->get_context_info(rdma_ib_info);
  handler->build_rdma_connection();
  printf("init done\n");
  char *a = "hello world";
  //   handler->write_with_imm(a, 12);
  return 0;
}
