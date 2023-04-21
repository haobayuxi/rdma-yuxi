#include <stdio.h>

#include "erpc/rpc.h"

static const std::string kServerHostname = "192.168.3.71";
static const std::string kClientHostname = "192.168.3.72";

static constexpr uint16_t kUDPPort = 31850;
static constexpr uint8_t kReqType = 2;
static constexpr size_t kMsgSize = 16;

// class Server {
//  public:
//   Server(int id, SystemConfig config) : server_id(id), config(config) {}

//   ~Server() {}

//   void GenThreads();

//   void InitData();

//   void InitRdma();

//  private:
//   const int server_id;
//   const SystemConfig config;
// };