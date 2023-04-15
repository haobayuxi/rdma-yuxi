
#include <json/json.h>

#include <fstream>
#include <iostream>
#include <string>

#include "common.h"

SystemConfig read_config_from_file() {
  // std::string config_file = "system_config.json";
  Json::Reader reader;
  Json::Value root;
  ifstream srcFile("system_config.json", ios::binary);
  if (!srcFile.is_open()) {
    std::cout << "file not open" << endl;
  }
  reader.parse(srcFile, root);

  struct SystemConfig sys_config;
  sys_config.executor_num = root["executor_num"];
  sys_config.server_addrs = root["server_addrs"];
  return sys_config;
}