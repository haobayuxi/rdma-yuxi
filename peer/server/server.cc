#include "server.h"

#include <iostream>

#include "common/json.h"

using namespace std;
int main(int argc, char* argv[]) {
  auto a = read_config_from_file();
  cout << a.executor_num << endl;
  return 0;
}