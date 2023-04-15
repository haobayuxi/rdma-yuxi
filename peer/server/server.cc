#include "server.h"

#include <iostream>

#include "common/json.h"

using namespace std;
int main(int argc, char* argv[]) {
  auto a = read_config_from_file();
  cout << a << endl;
  return 0;
}