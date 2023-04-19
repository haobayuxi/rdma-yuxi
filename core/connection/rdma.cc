

#include "rdma.h"

void init_client(rdma_fd *handler, char *server) {
  // char *server = "tstore04";
  uint16_t port = 9001;
  int sock = client_exchange(server, port);
  handler->fd = sock;
  context_info *ib_info = (context_info *)malloc(sizeof(context_info));
  open_device_and_alloc_pd(ib_info);
  get_context_info(handler, ib_info);
  build_rdma_connection(handler);
}

void init_server(rdma_fd *handler) {
  uint16_t port = 9001;
  int sock = server_exchange(port);
  handler->fd = sock;
  context_info *ib_info = (context_info *)malloc(sizeof(context_info));
  open_device_and_alloc_pd(ib_info);
  get_context_info(handler, ib_info);
  build_rdma_connection(handler);
}

bool client_send(rdma_fd *handler, char *local_buf, uint32_t size) {
  char *first_section = (char *)malloc(sizeof(char) * 5);
  memcpy(first_section, (char *)&size, sizeof(uint32_t));
  *(first_section + 4) = '1';
  rdma_write(handler, first_section, 5);
  rdma_write(handler, local_buf, size + 1);
  return true;
}

char *client_recv(rdma_fd *handler) {
  // use rdma read to get a msg
  return read_msg(handler);
}

bool server_send(rdma_fd *handler, char *local_buf, uint32_t size) {
  char *first_section = (char *)malloc(sizeof(char) * 5);
  memcpy(first_section, (char *)&size, sizeof(uint32_t));
  *(first_section + 4) = '1';
  rdma_write(handler, first_section, 5);
  rdma_write(handler, local_buf, size + 1);
  return true;
}

char *server_recv(rdma_fd *handler) { return read_msg(handler); }

void malloc_buf(long size) {}

void free_buf(long size) {}