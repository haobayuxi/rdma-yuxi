#include "server.h"

#include <iostream>
#include <thread>

using namespace std;

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

erpc::Rpc<erpc::CTransport> *rpc;

void req_handler(erpc::ReqHandle *req_handle, void *) {
  auto &resp = req_handle->pre_resp_msgbuf_;
  rpc->resize_msg_buffer(&resp, kMsgSize);
  sprintf(reinterpret_cast<char *>(resp.buf_), "hello");
  rpc->enqueue_response(req_handle, &resp);
}

int main() {
  std::string server_uri = kServerHostname + ":" + std::to_string(kUDPPort);
  erpc::Nexus nexus(server_uri);
  nexus.register_req_func(kReqType, req_handler);

  rpc = new erpc::Rpc<erpc::CTransport>(&nexus, nullptr, 0, nullptr);
  rpc->run_event_loop(100000);
}