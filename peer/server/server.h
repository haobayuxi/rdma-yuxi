#include "common/common.h"

class Server {
 public:
  Server(int id, SystemConfig config) : server_id(id), config(config) {}

  ~Server() {}

  void GenThreads();

  void InitData();

 private:
  const int server_id;
  const SystemConfig config;
};