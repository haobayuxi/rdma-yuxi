#include "server.h"

#include <iostream>

#include "common/json.h"

using namespace std;

void Server::InitData() {}

void Server::GenThreads() {
  auto thread_num = config.executor_num + net_handler_thread_num;

  auto thread_arr = new std::thread[thread_num];
  auto t_id_t t_id = 0;

  for (; t_id < net_handler_thread_num; t_id++) {
    thread_arr[i] = std::thread(run_thread, &param_arr[t_id], tatp_client,
                                smallbank_client, tpcc_client);
    /* Pin thread i to hardware thread i */
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(t_id, &cpuset);
    int rc = pthread_setaffinity_np(thread_arr[t_id].native_handle(),
                                    sizeof(cpu_set_t), &cpuset);
    if (rc != 0) {
      RDMA_LOG(WARNING) << "Error calling pthread_setaffinity_np: " << rc;
    }
  }

  for (; t_id < thread_num; t_id++) {
    thread_arr[i] = std::thread(run_thread, &param_arr[t_id], tatp_client,
                                smallbank_client, tpcc_client);
    /* Pin thread i to hardware thread i */
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(t_id, &cpuset);
    int rc = pthread_setaffinity_np(thread_arr[t_id].native_handle(),
                                    sizeof(cpu_set_t), &cpuset);
    if (rc != 0) {
      cout << "Error calling pthread_setaffinity_np: " << rc;
    }
  }

  for (t_id_t i = 0; i < thread_num; i++) {
    if (thread_arr[i].joinable()) {
      thread_arr[i].join();
    }
  }
  cout << "done" << endl;
}

int main(int argc, char* argv[]) {
  auto a = read_config_from_file();
  cout << a.executor_num << endl;
  return 0;
}