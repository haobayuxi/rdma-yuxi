
#include <arpa/inet.h>
#include <fcntl.h>
#include <getopt.h>
#include <infiniband/verbs.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include <string>

#define DEPTH 100
#define NUM_RTTS 100
#define TICKS_PER_USEC 2400
#define M_RC 0x0
#define M_UC 0x1
#define M_UD 0x2

#define FILL(st)                \
  do {                          \
    memset(&st, 0, sizeof(st)); \
  } while (0);

struct exchange_info {
  int lid;
  int qpn;
  int psn;
};

struct private_data {
  uint64_t buffer_addr;
  uint32_t buffer_rkey;
  size_t buffer_length;
};

struct context_info {
  struct ibv_device *dev;
  struct ibv_context *context;
  struct ibv_pd *pd;
};

class Handler {
 public:
  int build_rdma_connection();
  int get_context_info(context_info *ib_info);
  void sync_qp_info();
  void create_cq_and_qp(int max_depth, enum ibv_qp_type qp_type);
  void init_qp();
  void write_with_imm(char *buf, size_t size);
  static inline void post_write(size_t size, size_t offset);
  void set_fd(int fd_);
  static inline int poll_send_cq();

 private:
  struct ibv_context *context;
  struct ibv_pd *pd;
  struct ibv_cq *send_cq;
  struct ibv_cq *recv_cq;
  struct ibv_qp *qp;
  struct ibv_mr *mr;
  struct exchange_info *l_qp_info, *r_qp_info;
  struct private_data *l_private_data, *r_private_data;
  void *buf;
  void *send_buf;
  void *receive_buf;
  size_t buf_size;
  size_t send_buf_size;
  size_t receive_buf_size;
  size_t have_send;
  size_t have_read;
  size_t write_offset;
  char *addr;
  int ib_port_base;
  int tcp_port;
  int fd;  // socket fd;
  int mode;
  bool is_on;  // to tell the fd is normal and can work or not;
  uint32_t qkey;
};

int server_exchange(const char *server, uint16_t port);

int client_exchange(const char *server, uint16_t port);

int open_device_and_alloc_pd(context_info *ib_info);

int listenOn(uint16_t port);

int acceptAt(int sock);

int dialTo(const std::string &remoteIP, uint16_t port);