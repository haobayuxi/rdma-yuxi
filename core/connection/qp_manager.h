#include "rlib/rdma_ctrl.hpp"

using namespace rdmaio;

#define REMOTE_NODE_NUM 3

struct RemoteNode {
  node_id_t node_id;
  std::string ip;
  int port;
};

class QP_Manager {
 public:
  QP_Manager();
  void build_connection();
  void send_msg(node_id_t node_id, char* msg, size_t size, uint32_t imm);
  RdmaCtrlPtr global_rdma_ctrl;
  std::vector<RemoteNode> remote_nodes;
  RNicHandler* opened_rnic;

 private:
  std::vector<RCQP*> send_qp;
  std::vector<RCQP*> recv_qp;
}