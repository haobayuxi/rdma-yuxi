#include "common/json.h"

class Client {
 public:
  Client(int id, SystemConfig config) : client_id(id), config(config) {}

  ~Client() {}

 private:
  const int client_id;
  const SystemConfig config;
};