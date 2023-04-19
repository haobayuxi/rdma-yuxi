

#include "qp_manager.h"

using namespace rdmaio;

QP_Manager::QP_Manager() {
  //   opened_rnic = global_rdma_ctrl->open_device(idx);
  global_rdma_ctrl = new RdmaCtrl(0, 10001);
  RdmaCtrl::DevIdx idx{.dev_id = 0,
                       .port_id = 1};  // using the first RNIC's first port
  c->open_thread_local_device(idx);
  remote_nodes.push_back(
      RempteNode{.node_id = 0, .ip = "192.168.3.72", .port = 10001});
}

void QP_Manager::build_connection() {
  MemoryAttr remote_hash_mr = meta_man->GetRemoteHashMR(remote_node.node_id);

  // Build QPs with one remote machine (this machine can be a primary or a
  // backup) Create the thread local queue pair
  MemoryAttr local_mr = global_rdma_ctrl->get_local_mr(CLIENT_MR_ID);
  RCQP* data_qp = global_rdma_ctrl->create_rc_qp(
      create_rc_idx(remote_node.node_id, (int)global_tid * 2), opened_rnic,
      &local_mr);
  // Queue pair connection, exchange queue pair info via TCP
  ConnStatus rc;
  do {
    rc = data_qp->connect(remote_node.ip, remote_node.port);
    if (rc == SUCC) {
      data_qp->bind_remote_mr(
          remote_hash_mr);  // Bind the hash mr as the default remote mr for
                            // convenient parameter passing
      send_qp.push_back(data_qp);
      // RDMA_LOG(INFO) << "Thread " << global_tid << ": Data QP connected! with
      // remote node: " << remote_node.node_id << " ip: " << remote_node.ip;
    }
    usleep(2000);
  } while (rc != SUCC);
}

void QP_Manager::send_msg(node_id_t node_id, char* msg, size_t size,
                          uint32_t imm) {
  auto qp = send_qp[node_id];
  auto address = qp->remote_mr_->addr;
  qp->remote_mr_->addr += size;
  qp->post_send(IBV_WR_RDMA_WRITE_WITH_IMM, msg, size, address,
                IBV_SEND_SIGNALED);
}