
#include <json/json.h>

#include "config.h"

SystemConfig read_config_from_file() {
  std::string config_file = "system_config.json";
  Json::Reader reader;
  Json::Value root;
  ifstream srcFile(config_file, ios::binary);
  reader.parse(srcFile, root);

  struct SystemConfig sys_config;
  return sys_config;
}