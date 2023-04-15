#pragma once

#include <unordered_map>

struct SystemConfig {
  int executor_num;
  unordered_map<int, String> server_addrs;
}
