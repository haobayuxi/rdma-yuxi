

// // refactor

// #include <arpa/inet.h>
// #include <getopt.h>
// #include <infiniband/verbs.h>
// #include <netdb.h>
// #include <netinet/in.h>
// #include <netinet/tcp.h>
// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <sys/socket.h>
// #include <sys/time.h>
// #include <sys/types.h>
// #include <time.h>
// #include <unistd.h>

// #include <string>

// #define DEPTH 100
// #define NUM_RTTS 100
// #define TICKS_PER_USEC 2400
// #define M_RC 0x0
// #define M_UC 0x1
// #define M_UD 0x2
// #define SOCKET_ERROR -1

// #define CHECK(val, msg)                 \
//   if (val) {                            \
//     printf("error: %d %s\n", val, msg); \
//     exit(-1);                           \
//   }

// #define CPEA(ret)                           \
//   if (!ret) {                               \
//     PRINT_LINE                              \
//     printf("ERROR: %s\n", strerror(errno)); \
//     printf("ERROR: NULL\n");                \
//     exit(1);                                \
//   }

// #define CPEN(ret)                             \
//   if (ret == NULL) {                          \
//     PRINT_LINE                                \
//     printf("ERROR: %s\n", strerror(h_errno)); \
//     printf("ERROR: NULL\n");                  \
//     exit(1);                                  \
//   }

// #define CPE(ret)                            \
//   if (ret) {                                \
//     PRINT_LINE                              \
//     printf("ERROR: %s\n", strerror(errno)); \
//     printf("ERROR CODE: %d\n", ret);        \
//     exit(ret);                              \
//   }

// #define FILL(st)                \
//   do {                          \
//     memset(&st, 0, sizeof(st)); \
//   } while (0);

// #define time_sec (time((time_t *)NULL))
// #define PRINT_TIME printf("time: %Lf\n", (long double)clock());
// #define PRINT_LINE printf("line: %d\n", __LINE__);
// #define PRINT_FUNC printf("func: %s\n", __FUNC__);

// static void m_nano_sleep(int nsec) {
//   struct timespec tim, tim2;
//   tim.tv_sec = 0;
//   tim.tv_nsec = nsec;
//   if (nanosleep(&tim, &tim2) < 0) {
//     printf("SLEEP ERROR!\n");
//     exit(1);
//   }
// }

// /* communication data structure */

// struct exchange_info {
//   int lid;
//   int qpn;
//   int psn;
// };

// struct private_data {
//   uint64_t buffer_addr;
//   uint32_t buffer_rkey;
//   size_t buffer_length;
// };

// struct context_info {
//   struct ibv_device *dev;
//   struct ibv_context *context;
//   struct ibv_pd *pd;
// };

// struct rdma_fd {
//   struct ibv_context *context;
//   struct ibv_pd *pd;
//   struct ibv_cq *send_cq;
//   struct ibv_cq *recv_cq;
//   struct ibv_qp *qp;
//   struct ibv_mr *mr;
//   struct exchange_info *l_qp_info, *r_qp_info;
//   struct private_data *l_private_data, *r_private_data;
//   void *buf;
//   void *send_buf;
//   void *receive_buf;
//   size_t buf_size;
//   size_t send_buf_size;
//   size_t receive_buf_size;
//   size_t have_send;
//   size_t have_read;
//   size_t write_offset;
//   char *addr;
//   int ib_port_base;
//   int tcp_port;
//   int fd;  // socket fd;
//   int mode;
//   bool is_on;  // to tell the fd is normal and can work or not;
//   uint32_t qkey;
// };

// /* some test interface */

// /* get rdma info */

// int open_device_and_alloc_pd(context_info *ib_info);
// int get_context_info(rdma_fd *handler, context_info *ib_info);
// /* communication interface */
// void query_qp(rdma_fd *handler);
// int rdma_connet(rdma_fd *handler);  // for rdma client to connet;
// int rdma_accept(rdma_fd *handler);  // server to accept  exchange qp info;

// int rdma_write(rdma_fd *handler, char *buf, size_t len);
// // int rdma_read(rdma_fd *handler, void *buf, size_t len);
// int read_msg(rdma_fd *handler);

// int build_rdma_connection(rdma_fd *handler);

// int listenOn(uint16_t port);

// int acceptAt(int sock);

// int dialTo(const std::string &remoteIP, uint16_t port);

// static inline int poll_send_cq(rdma_fd *handler) {
//   struct ibv_wc wc;
//   // printf("handler addr: %p, handler->send_cq addr: %p\n", handler,
//   // handler->send_cq);
//   while (ibv_poll_cq(handler->send_cq, 1, &wc) < 1)
//     ;
//   if (wc.status != IBV_WC_SUCCESS) {
//     printf("Status: %d\n", wc.status);
//     printf("Ibv_poll_cq error!\n");
//     printf("Error: %s\n", strerror(errno));
//     return -1;
//   }
//   //    printf("poll cq success！\n");
//   return 0;
// }

// static inline void poll_recv_cq(rdma_fd *handler) {
//   struct ibv_wc wc;

//   while (ibv_poll_cq(handler->recv_cq, 1, &wc) < 1)
//     ;
//   if (wc.status != IBV_WC_SUCCESS) {
//     printf("Ibv_poll_cq error!\n");
//     printf("Error: %s\n", strerror(errno));
//     return;
//   }
//   printf("poll cq success！\n");
// }
