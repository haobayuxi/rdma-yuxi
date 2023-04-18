#include "client.h"

#include <assert.h>
#include <stdio.h>

#include <chrono>
#include <ctime>

#include "rlib/rdma_ctrl.hpp"

using namespace rdmaio;
using namespace std::chrono;

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
  RdmaCtrl *c = new RdmaCtrl(client_node_id, tcp_port);
  RdmaCtrl::DevIdx idx{.dev_id = 0,
                       .port_id = 1};  // using the first RNIC's first port
  c->open_thread_local_device(idx);

  // register a buffer to the previous opened device, using id = 73
  char *buffer = (char *)malloc(4096);
  memset(buffer, 0, 4096);
  RDMA_ASSERT(c->register_memory(MR_ID, buffer, 4096, c->get_device()) == true);

  // get remote server's memory information
  MemoryAttr remote_mr;
  while (QP::get_remote_mr("192.168.3.72", server_port, MR_ID, &remote_mr) !=
         SUCC) {
    usleep(2000);
  }

  // create the RC qp to access remote server's memory, using the previous
  // registered memory
  MemoryAttr local_mr = c->get_local_mr(MR_ID);
  RCQP *qp = c->create_rc_qp(create_rc_idx(1, 0), c->get_device(), &local_mr);
  qp->bind_remote_mr(remote_mr);  // bind to the previous allocated mr

  while (qp->connect("192.168.3.72", server_port) != SUCC) {
    usleep(2000);
  }

  printf("client: QP connected!\n");
  ibv_wc wc;
  char *local_buf = buffer;
  uint64_t address = 0;
  int msg_len = 11;  // length of "hello world"

  // read an uint64_t from the server
  for (int i = 0; i < 100; i++) {
    auto start_time = system_clock::now();
    auto rc = qp->post_send(IBV_WR_RDMA_READ, local_buf, msg_len, address,
                            IBV_SEND_SIGNALED);
    //   if (rc == SUCC) {
    //     printf("client: post ok\n");
    //   } else {
    //     printf("client: post fail. rc=%d\n", rc);
    //   }
    rc = qp->poll_till_completion(wc, no_timeout);
    // then get the results, stored in the local_buffer
    if (rc == SUCC) {
      auto end_time = system_clock::now();
      auto microseconds_since_epoch =
          duration_cast<microseconds>(end_time - start_time)
              .count();  // 将时长转换为微秒数
      printf("client: poll ok %d\n", microseconds_since_epoch);
      printf("msg read: %s\n", local_buf);
    } else {
      printf("client: poll fail. rc=%d\n", rc);
    }
  }

  return 0;
}
