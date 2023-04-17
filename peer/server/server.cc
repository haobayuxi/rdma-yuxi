#include "server.h"

#include <iostream>
#include <thread>

#include "common/common.h"
#include "common/json.h"
#include "rlib/rdma_ctrl.hpp"

using namespace std;
using namespace rdmaio;

void Server::InitData() {}

void Server::GenThreads() {
  // auto thread_num = config.executor_num + net_handler_thread_num;

  // auto thread_arr = new std::thread[thread_num];
  // t_id_t t_id = 0;

  // for (; t_id < net_handler_thread_num; t_id++) {
  //   thread_arr[t_id] = std::thread(run_thread, &param_arr[t_id], tatp_client,
  //                                  smallbank_client, tpcc_client);
  //   /* Pin thread i to hardware thread i */
  //   cpu_set_t cpuset;
  //   CPU_ZERO(&cpuset);
  //   CPU_SET(t_id, &cpuset);
  //   int rc = pthread_setaffinity_np(thread_arr[t_id].native_handle(),
  //                                   sizeof(cpu_set_t), &cpuset);
  //   if (rc != 0) {
  //     RDMA_LOG(WARNING) << "Error calling pthread_setaffinity_np: " << rc;
  //   }
  // }

  // for (; t_id < thread_num; t_id++) {
  //   thread_arr[t_id] = std::thread(run_thread, &param_arr[t_id], tatp_client,
  //                                  smallbank_client, tpcc_client);
  //   /* Pin thread i to hardware thread i */
  //   cpu_set_t cpuset;
  //   CPU_ZERO(&cpuset);
  //   CPU_SET(t_id, &cpuset);
  //   int rc = pthread_setaffinity_np(thread_arr[t_id].native_handle(),
  //                                   sizeof(cpu_set_t), &cpuset);
  //   if (rc != 0) {
  //     cout << "Error calling pthread_setaffinity_np: " << rc;
  //   }
  // }

  // for (t_id_t i = 0; i < thread_num; i++) {
  //   if (thread_arr[i].joinable()) {
  //     thread_arr[i].join();
  //   }
  // }
  // cout << "done" << endl;
}

void Server::InitRdma() {}

int main(int argc, char *argv[]) {
  int server_node_id = 1;
  int tcp_port = 10001;

  auto rdma_ib_info = (context_info *)malloc(sizeof(context_info));
  open_device_and_alloc_pd(rdma_ib_info);

  auto listener = listenOn(tcp_port);
  auto fd = acceptAt(listener);
  rdma_fd *handler = (rdma_fd *)malloc(sizeof(rdma_fd));
  handler->fd = fd;

  get_context_info(handler, rdma_ib_info);
  build_rdma_connection(handler);
  printf("connection complete!\n");
  poll_recv_cq(handler);
  while (1) {
    sleep(1);
  }
  // RdmaCtrl *c = new RdmaCtrl(server_node_id, tcp_port);
  // RdmaCtrl::DevIdx idx{.dev_id = 0,
  //                      .port_id = 1};  // using the first RNIC's first port
  // c->open_thread_local_device(idx);

  // // register a buffer to the previous opened device, using id = 73
  // char *buffer = (char *)malloc(4096);
  // memset(buffer, 0, 4096);
  // RDMA_ASSERT(c->register_memory(MR_ID, buffer, 4096, c->get_device()) ==
  // true);

  // char s[] = "hello world";
  // memcpy(buffer, s, strlen(s));

  // // MemoryAttr local_mr = c->get_local_mr(MR_ID);
  // // RCQP *qp = c->create_rc_qp(create_rc_idx(1, 0), c->get_device(),
  // // &local_mr);

  // // // server also needs to "connect" clinet.
  // // while (qp->connect("localhost", client_port, create_rc_idx(1, 0)) !=
  // SUCC)
  // // {
  // //   usleep(2000);
  // // }

  // printf("server: QP init!\n");
  // while (true) {
  //   // This is RDMA, server does not need to do anything :)
  //   sleep(1);
  // }

  return 0;
}