

#include "rdma_transport.h"

bool client_send(rdma_fd *handler, char *local_buf, uint32_t size);

char *client_recv(rdma_fd *handler);

bool server_send(rdma_fd *handler, char *local_buf, uint32_t size);

char *server_recv(rdma_fd *handler);

void malloc_buf(long size);

void free_buf(long size);

// init client with host name
void init_client(rdma_fd *handler, char *server);
void init_server(rdma_fd *handler);
