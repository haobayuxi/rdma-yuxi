#include "common/json.h"
// #include "connection/handler.h"
#include "connection/rdma.h"

class Client {
 public:
  Client(int id, SystemConfig config) : client_id(id), config(config) {}

  ~Client() {}

 private:
  const int client_id;
  const SystemConfig config;
};