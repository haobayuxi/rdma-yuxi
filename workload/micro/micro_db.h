
#include "unordered_map"

using namespace std;

class micro_db {
 private:
  unordered_map<int, int> table;

 public:
  micro_db(/* args */);
  ~micro_db();
};

micro_db::micro_db(/* args */) {}

micro_db::~micro_db() {}
