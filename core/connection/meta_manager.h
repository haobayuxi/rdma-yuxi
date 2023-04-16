// Author: Ming Zhang
// Copyright (c) 2022

#pragma once

#include <atomic>
#include <unordered_map>

#include "rlib/rdma_ctrl.hpp"

using namespace rdmaio;

// const size_t LOG_BUFFER_SIZE = 1024 * 1024 * 512;

struct RemoteNode {
  node_id_t node_id;
  std::string ip;
  int port;
};

class MetaManager {
 public:
  MetaManager();

 private:
  node_id_t local_machine_id;

 public:
  // Used by QP manager and RDMA Region
  RdmaCtrlPtr global_rdma_ctrl;

  std::vector<RemoteNode> remote_nodes;
  RemoteNode cto;

  RNicHandler* opened_rnic;

  // Below are some parameteres from json file
  int64_t txn_system;
};
