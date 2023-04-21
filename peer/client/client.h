#include <stdio.h>

#include "rpc.h"

static const std::string kServerHostname = "192.168.3.71";
static const std::string kClientHostname = "192.168.3.72";

static constexpr uint16_t kUDPPort = 31850;
static constexpr uint8_t kReqType = 2;
static constexpr size_t kMsgSize = 16;

// class Client {
//  public:
//   Client(int id, SystemConfig config) : client_id(id), config(config) {}

//   ~Client() {}

//  private:
//   const int client_id;
//   const SystemConfig config;
// };