#include "common/common.h"

class Server {
 public:
  Server(int id, SystemConfig config) : server_id(id), config(config) {}

  ~Server() {}

  Void InitData() private : const int server_id;
  const SystemConfig config;
};