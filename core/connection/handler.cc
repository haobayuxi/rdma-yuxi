

#include "handler.h"

#include <pthread.h>
#include <sys/time.h>
#include <time.h>

#define MSG_SIZE 1024

int listenOn(uint16_t port) {
  struct sockaddr_in addr;
  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htons(INADDR_ANY);
  addr.sin_port = htons(port);
  int _socket = socket(PF_INET, SOCK_STREAM, 0);
  if (_socket < 0) return _socket;
  int keepAlive = 1;
  int keepIdle = 3;
  int keepInterval = 2;
  int keepCount = 3;
  if (setsockopt(_socket, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepAlive,
                 sizeof(keepAlive)) == SOCKET_ERROR)
    fprintf(stderr, "Cannot set socket keep alive: %d\n", errno);
  if (setsockopt(_socket, SOL_TCP, TCP_KEEPIDLE, (void *)&keepIdle,
                 sizeof(keepIdle)) == SOCKET_ERROR)
    fprintf(stderr, "Cannot set socket keep Idle: %d\n", errno);
  if (setsockopt(_socket, SOL_TCP, TCP_KEEPINTVL, (void *)&keepInterval,
                 sizeof(keepInterval)) == SOCKET_ERROR)
    fprintf(stderr, "Cannot set socket keep Interval: %d\n", errno);
  if (setsockopt(_socket, SOL_TCP, TCP_KEEPCNT, (void *)&keepCount,
                 sizeof(keepCount)) == SOCKET_ERROR)
    fprintf(stderr, "Cannot set socket keep CNT: %d\n", errno);
  if (bind(_socket, (struct sockaddr *)&addr, sizeof(addr))) return -1;
  if (listen(_socket, 2)) return -1;
  return _socket;
}

int acceptAt(int sock) {
  struct sockaddr_in remoteAddr;
  socklen_t length = sizeof(remoteAddr);
  int keepAlive = 1;
  int keepIdle = 3;
  int keepInterval = 2;
  int keepCount = 3;
  int nsock = accept(sock, (struct sockaddr *)&remoteAddr, &length);
  if (setsockopt(nsock, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepAlive,
                 sizeof(keepAlive)) == SOCKET_ERROR)
    fprintf(stderr, "Cannot set socket keep alive: %d\n", errno);
  if (setsockopt(nsock, SOL_TCP, TCP_KEEPIDLE, (void *)&keepIdle,
                 sizeof(keepIdle)) == SOCKET_ERROR)
    fprintf(stderr, "Cannot set socket keep Idle: %d\n", errno);
  if (setsockopt(nsock, SOL_TCP, TCP_KEEPINTVL, (void *)&keepInterval,
                 sizeof(keepInterval)) == SOCKET_ERROR)
    fprintf(stderr, "Cannot set socket keep Interval: %d\n", errno);
  if (setsockopt(nsock, SOL_TCP, TCP_KEEPCNT, (void *)&keepCount,
                 sizeof(keepCount)) == SOCKET_ERROR)
    fprintf(stderr, "Cannot set socket keep CNT: %d\n", errno);
  // set_nonblocking(nsock);
  return nsock;
}

int dialTo(const std::string &remoteIP, uint16_t port) {
  int localSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (localSocket < 0) return localSocket;
  struct sockaddr_in remoteAddr;
  bzero(&remoteAddr, sizeof(remoteAddr));
  remoteAddr.sin_family = AF_INET;
  if (inet_aton(remoteIP.c_str(), &remoteAddr.sin_addr) == 0) return -1;
  remoteAddr.sin_port = htons(port);
  socklen_t remoteAddrLength = sizeof(remoteAddr);
  if (connect(localSocket, (struct sockaddr *)&remoteAddr, remoteAddrLength) <
      0)
    return -1;
  int keepAlive = 1;
  int keepIdle = 3;
  int keepInterval = 2;
  int keepCount = 3;
  if (setsockopt(localSocket, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepAlive,
                 sizeof(keepAlive)) == SOCKET_ERROR)
    fprintf(stderr, "Cannot set socket keep alive: %d\n", errno);
  if (setsockopt(localSocket, SOL_TCP, TCP_KEEPIDLE, (void *)&keepIdle,
                 sizeof(keepIdle)) == SOCKET_ERROR)
    fprintf(stderr, "Cannot set socket keep Idle: %d\n", errno);
  if (setsockopt(localSocket, SOL_TCP, TCP_KEEPINTVL, (void *)&keepInterval,
                 sizeof(keepInterval)) == SOCKET_ERROR)
    fprintf(stderr, "Cannot set socket keep Interval: %d\n", errno);
  if (setsockopt(localSocket, SOL_TCP, TCP_KEEPCNT, (void *)&keepCount,
                 sizeof(keepCount)) == SOCKET_ERROR)
    fprintf(stderr, "Cannot set socket keep CNT: %d\n", errno);
  // set_nonblocking(localSocket);
  return localSocket;
}

ssize_t readUntil1(int sock, void *buf, size_t len) {
  ssize_t readed = (ssize_t)len;
  while (readed > 0) {
    ssize_t cur = read(sock, buf, (size_t)readed);
    if (cur < 0) {
      if (errno != EINTR) {
        fprintf(stdout, "Connection error! Cannot read from sock %d!\n", sock);
        return cur;
      } else {
        printf("help me!\n");
        continue;
      }
    }
    readed -= cur;
  }
  return readed;
}

long sendData1(int sock, void *buf, size_t len) {
  ssize_t sent = 0;
  while (len > 0) {
    ssize_t realWriteCount = write(sock, buf + sent, len);
    if (realWriteCount < 0) {
      if (errno != EINTR) {
        fprintf(stderr,
                "Connection error! Send to sock %d with %ld write count!\n",
                sock, realWriteCount);
        return realWriteCount;
      } else {
        printf("help!\n");
        continue;
      }
    }
    len -= realWriteCount;
    sent += realWriteCount;
  }
  return sent;
}

static struct ibv_device *get_ib_device(int index) {
  struct ibv_device **devices;
  int num;
  devices = ibv_get_device_list(&num);
  if (index >= num) {
    printf("Not have the deivce\n");
    return NULL;
  }
  return devices[index];
}

static int get_lid(rdma_fd *handler) {
  struct ibv_port_attr port_attr;
  CPEN(handler->context);
  CPE(ibv_query_port(handler->context, handler->ib_port_base, &port_attr));
  return port_attr.lid;
}

int open_device_and_alloc_pd(context_info *ib_info) {
  struct ibv_device *dev = get_ib_device(0);
  ib_info->dev = dev;
  ib_info->context = ibv_open_device(dev);
  CPEN(ib_info);
  ib_info->pd = ibv_alloc_pd(ib_info->context);
  CPEN(ib_info->context);
  return 0;
}

int get_context_info(rdma_fd *handler, context_info *ib_info) {
  handler->context = ib_info->context;
  //    handler->dev = ib_info->dev;
  handler->pd = ib_info->pd;
  return 0;
}

static void reg_buffer(rdma_fd *handler) {
  int flags = IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE |
              IBV_ACCESS_REMOTE_READ | IBV_ACCESS_REMOTE_ATOMIC;
  if (handler->mr == NULL) {
    handler->mr =
        ibv_reg_mr(handler->pd, handler->buf, handler->buf_size, flags);
    CPEN(handler->mr);
  } else {
    printf("Already register\n");
    return;
  }
}

static void init_qp(rdma_fd *handler) {
  struct ibv_qp_attr qp_attr;
  FILL(qp_attr);
  qp_attr.qp_state = IBV_QPS_INIT;
  qp_attr.port_num = handler->ib_port_base;
  qp_attr.pkey_index = 0;
  if (handler->mode == M_UD) {
    qp_attr.qkey = handler->qkey;
  } else {
    qp_attr.qp_access_flags = IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE |
                              IBV_ACCESS_REMOTE_READ | IBV_ACCESS_REMOTE_ATOMIC;
  }
  int flags = IBV_QP_STATE | IBV_QP_PKEY_INDEX | IBV_QP_PORT;
  if (handler->mode == M_UD) {
    flags |= IBV_QP_QKEY;
  } else {
    flags |= IBV_QP_ACCESS_FLAGS;
  }
  //	printf("come here!\n");
  CPE(ibv_modify_qp(handler->qp, &qp_attr, flags));
  //	printf("QPNum = %d\n", handler->qp->qp_num);
}

static void create_cq_and_qp(rdma_fd *handler, int max_depth,
                             enum ibv_qp_type qp_type) {
  handler->send_cq = NULL;
  handler->recv_cq = NULL;
  handler->qp = NULL;

  handler->send_cq = ibv_create_cq(handler->context, max_depth, NULL, NULL, 0);
  CPEN(handler->send_cq);
  handler->recv_cq = ibv_create_cq(handler->context, max_depth, NULL, NULL, 0);
  CPEN(handler->recv_cq);

  struct ibv_qp_init_attr qp_init_attr;
  FILL(qp_init_attr);
  qp_init_attr.send_cq = handler->send_cq;
  qp_init_attr.recv_cq = handler->recv_cq;
  qp_init_attr.cap.max_send_wr = max_depth;
  qp_init_attr.cap.max_recv_wr = max_depth;

  qp_init_attr.cap.max_send_sge = 1;
  qp_init_attr.cap.max_recv_sge = 1;
  qp_init_attr.cap.max_inline_data = 64;
  qp_init_attr.qp_type = qp_type;
  qp_init_attr.sq_sig_all = 0;
  handler->qp = ibv_create_qp(handler->pd, &qp_init_attr);
  CPEN(handler->qp);
  init_qp(handler);
}

static void modify_qp_to_rts_and_rtr(rdma_fd *handler) {
  struct ibv_qp_attr qp_attr;
  FILL(qp_attr);
  int flags;
  qp_attr.qp_state = IBV_QPS_RTR;
  if (handler->mode == M_UD) {
    flags = IBV_QP_STATE;
  } else {
    flags = IBV_QP_STATE | IBV_QP_AV | IBV_QP_PATH_MTU | IBV_QP_DEST_QPN |
            IBV_QP_RQ_PSN;
    qp_attr.path_mtu = IBV_MTU_1024;
    qp_attr.dest_qp_num = handler->r_qp_info->qpn;
    qp_attr.rq_psn = handler->r_qp_info->psn;
    if (handler->mode == M_RC) {
      qp_attr.max_dest_rd_atomic = 1;
      qp_attr.min_rnr_timer = 12;
      flags |= IBV_QP_MAX_DEST_RD_ATOMIC | IBV_QP_MIN_RNR_TIMER;
    }
    qp_attr.ah_attr.is_global = 0;
    qp_attr.ah_attr.dlid = handler->r_qp_info->lid;
    qp_attr.ah_attr.sl = 0;
    qp_attr.ah_attr.src_path_bits = 0;
    qp_attr.ah_attr.port_num = handler->ib_port_base;
  }
  printf("IB port: %d\n", handler->ib_port_base);
  CPE(ibv_modify_qp(handler->qp, &qp_attr, flags));

  qp_attr.qp_state = IBV_QPS_RTS;
  flags = IBV_QP_STATE | IBV_QP_SQ_PSN;
  if (handler->mode == M_UD) {
    qp_attr.sq_psn = lrand48() & 0xffffff;
  } else {
    qp_attr.sq_psn = handler->l_qp_info->psn;
    if (handler->mode == M_RC) {
      qp_attr.timeout = 14;
      qp_attr.retry_cnt = 7;
      qp_attr.rnr_retry = 7;
      qp_attr.max_rd_atomic = 1;
      flags |= IBV_QP_TIMEOUT | IBV_QP_RETRY_CNT | IBV_QP_RNR_RETRY |
               IBV_QP_MAX_QP_RD_ATOMIC;
    }
  }
  CPE(ibv_modify_qp(handler->qp, &qp_attr, flags));
}

static void exchange(rdma_fd *handler) {
  ssize_t ret;
  ret = sendData1(handler->fd, handler->l_qp_info, sizeof(exchange_info));
  ret = readUntil1(handler->fd, handler->r_qp_info, sizeof(exchange_info));
  //	printf("remote lid %d, qpn %d\n", handler->r_qp_info->lid,
  // handler->r_qp_info->qpn);
  ret = sendData1(handler->fd, handler->l_private_data, sizeof(private_data));
  ret = readUntil1(handler->fd, handler->r_private_data, sizeof(private_data));
  //	printf("remote addr %ld, rkey %d\n",
  // handler->r_private_data->buffer_addr,
  // handler->r_private_data->buffer_rkey);
}

static void sync_qp_info(rdma_fd *handler) {
  srand(time(NULL));
  handler->l_qp_info = (exchange_info *)malloc(sizeof(struct exchange_info));
  handler->l_qp_info->psn = lrand48() & 0xffffff;
  handler->l_qp_info->qpn = handler->qp->qp_num;
  handler->l_qp_info->lid = get_lid(handler);
  handler->l_private_data = (private_data *)malloc(sizeof(private_data));
  handler->l_private_data->buffer_addr = (uint64_t)handler->buf;
  handler->l_private_data->buffer_rkey = handler->mr->rkey;
  handler->l_private_data->buffer_length = handler->mr->length;
  handler->r_qp_info = (exchange_info *)malloc(sizeof(exchange_info));
  handler->r_private_data = (private_data *)malloc(sizeof(private_data));

  printf("Local LID = %d, QPN = %d, PSN = %d\n", handler->l_qp_info->lid,
         handler->l_qp_info->qpn, handler->l_qp_info->psn);
  printf("Local Addr = %ld, RKey = %d, LEN = %zu\n",
         handler->l_private_data->buffer_addr,
         handler->l_private_data->buffer_rkey,
         handler->l_private_data->buffer_length);

  //		struct m_param param;
  // 		if (ibv_res->is_server) {
  // //				ibv_res->rparam =
  // m_server_exchange(ibv_res->port, ibv_res->lparam);
  // server_exchange(ibv_res->port, ibv_res->lparam, ibv_res->lpriv_data,
  // &ibv_res->rparam, &ibv_res->rpriv_data); 		} else {
  // 				client_exchange(server, ibv_res->port,
  // ibv_res->lparam,
  // ibv_res->lpriv_data,
  // &ibv_res->rparam, &ibv_res->rpriv_data);
  // //				ibv_res->rparam = m_client_exchange(server,
  // ibv_res->port, ibv_res->lparam);
  // 		}
  exchange(handler);
  printf("Remote LID = %d, QPN = %d, PSN = %d\n", handler->r_qp_info->lid,
         handler->r_qp_info->qpn, handler->r_qp_info->psn);

  printf("Remote Addr = %ld, RKey = %d, LEN = %zu\n",
         handler->r_private_data->buffer_addr,
         handler->r_private_data->buffer_rkey,
         handler->r_private_data->buffer_length);
}

static inline void post_write(rdma_fd *handler, size_t size, size_t offset) {
  struct ibv_sge sge = {(uint64_t)handler->buf + offset, (uint32_t)size,
                        handler->mr->lkey};
  struct ibv_send_wr send_wr;
  FILL(send_wr);
  send_wr.wr_id = 1;
  send_wr.next = NULL;
  send_wr.sg_list = &sge;
  send_wr.num_sge = 1;
  send_wr.opcode = IBV_WR_RDMA_WRITE;
  send_wr.wr.rdma.remote_addr =
      handler->r_private_data->buffer_addr + handler->send_buf_size + offset;
  send_wr.wr.rdma.rkey = handler->r_private_data->buffer_rkey;
  send_wr.send_flags = IBV_SEND_SIGNALED;

  struct ibv_send_wr *bad_send_wr;
  CPE(ibv_post_send(handler->qp, &send_wr, &bad_send_wr));
}

// static inline void post_write_unsig (rdma_fd *handler, size_t size,  size_t
// offset) { 	struct ibv_sge sge = { 		(uint64_t)handler->buf + offset,
// (uint32_t)size, handler->mr->lkey
// 	};
// 	struct ibv_send_wr send_wr;
// 	FILL(send_wr);
// 	send_wr.wr_id = 1;
// 	send_wr.next = NULL;
// 	send_wr.sg_list = &sge;
// 	send_wr.num_sge = 1;
// 	send_wr.opcode = IBV_WR_RDMA_WRITE;
// 	send_wr.wr.rdma.remote_addr = handler->r_private_data->buffer_addr+
// handler->send_buf_size + offset; 	send_wr.wr.rdma.rkey =
// handler->r_private_data->buffer_rkey;

// 	struct ibv_send_wr *bad_send_wr;
// 	CPE(ibv_post_send(handler->qp, &send_wr, &bad_send_wr));
// }

// int counter = 0;

int rdma_write(rdma_fd *handler, char *buf, size_t size) {
  int ret = 0;
  if (handler->write_offset + size > handler->send_buf_size) {
    usleep(10);
    handler->write_offset = 0;
    handler->have_send = 0;
  }
  memcpy(handler->buf + handler->write_offset, buf, size);
  post_write(handler, size, handler->have_send);
  // if(counter % 20 == 0){
  // 	ret = poll_send_cq(handler);
  // 	if(ret == -1){
  // 	    printf("poll cq failed\n");
  // 	}
  // }
  ret = poll_send_cq(handler);
  // printf("[buf size]: %d\n", size);
  // printf("[buf data]:");
  // for(int i = 0;  i < size; i++){
  // 	printf("%02x ", *(char *)(handler->buf+handler->write_offset + i));
  // }
  // printf("\n");
  if (ret != -1) {
    handler->write_offset += size;
    handler->have_send += size;
  }
  printf("write done\n");
  return ret;
}

/*
int rdma_write(rdma_fd *handler, char *buf, size_t size){
        int ret;

    if(handler->write_offset + size > handler->send_buf_size){
        usleep(10);
                post_write(handler, MSG_SIZE, handler->have_send);
                handler->write_offset  = 0;
                handler->have_send = 0;
        }
        memcpy(handler->buf + handler->write_offset, buf, size);
//	for(int i = 0; i < size; i++){
//		printf("%02x ",  buf[i]);
//	}
        printf("\n");
        handler->write_offset += size;
    ret = 0;
        if(handler->write_offset - handler->have_send >= MSG_SIZE){
                printf("send out\n");
                post_write(handler, MSG_SIZE, handler->have_send);
                handler->have_send += MSG_SIZE;
                printf("have_send: %d\n", handler->have_send);
                ret = poll_send_cq(handler);
        }
    return ret;
}
*/

int read_msg(rdma_fd *handler) {
  if (handler->have_read + sizeof(uint32_t) + 1 > handler->receive_buf_size) {
    handler->have_read = 0;
  }
  uint32_t msg_size;
  while (1) {
    volatile char *end =
        (volatile char *)(handler->receive_buf + handler->have_read + 4);

    if (*end == '1') {
      break;
    }
  }
  memcpy(&msg_size, handler->receive_buf + handler->have_read,
         sizeof(uint32_t));
  //	printf("%d\n", msg_size);
  memset(handler->receive_buf + handler->have_read, 0, 5);
  handler->have_read += sizeof(uint32_t) + 1;
  if (handler->have_read + msg_size + 1 > handler->receive_buf_size) {
    handler->have_read = 0;
  }
  while (1) {
    volatile char *end =
        (volatile char *)(handler->receive_buf + handler->have_read + msg_size);
    if (*end == '1') {
      break;
    }
  }
  char *msg = (char *)malloc(msg_size * sizeof(char));
  memcpy(msg, handler->receive_buf + handler->have_read, msg_size);
  memset(handler->receive_buf + handler->have_read, 0, msg_size + 1);
  handler->have_read += msg_size + 1;
  // for(int i = 0; i < msg_size; i++){
  //  	printf("%02x ", msg[i]);
  // }
  //    printf("\n");
  //    printf("have_read:%d\n", handler->have_read);
}

void query_qp(rdma_fd *handler) {
  struct ibv_qp_attr attr;
  struct ibv_qp_init_attr init_attr;

  if (ibv_query_qp(handler->qp, &attr, IBV_QP_STATE, &init_attr)) {
    fprintf(stdout, "Failed to query QP state\n");
    return;
  }
  printf("QP STATE:%d\n", attr.cur_qp_state);
}

int client_exchange(const char *server, uint16_t port) {
  int s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (s == -1) {
    printf("SOCK ERROR!\n");
  }
  struct hostent *hent = gethostbyname(server);
  CPEN(hent);
  ssize_t temp;
  struct sockaddr_in sin;
  FILL(sin);
  sin.sin_family = PF_INET;
  sin.sin_port = htons(port);
  sin.sin_addr = *((struct in_addr *)hent->h_addr);
  m_nano_sleep(50000000);
  CPE((connect(s, (struct sockaddr *)&sin, sizeof(sin)) == -1));
  return s;
}

int server_exchange(uint16_t port) {
  int s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (s == -1) {
    printf("SOCK ERROR!\n");
    exit(1);
  }
  int on = 1;
  CPE((setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof on) == -1));
  struct sockaddr_in sin;
  FILL(sin);
  sin.sin_family = PF_INET;
  sin.sin_port = htons(port);
  sin.sin_addr.s_addr = htons(INADDR_ANY);
  CPE((bind(s, (struct sockaddr *)&sin, sizeof(sin)) == -1));
  CPE((listen(s, 1) == -1));
  struct sockaddr_in csin;
  socklen_t csinsize = sizeof(csin);
  int c = accept(s, (struct sockaddr *)&csin, &csinsize);
  CPE((c == -1));
  return c;
}

int build_rdma_connection(rdma_fd *handler) {
  handler->mode = M_RC;
  handler->ib_port_base = 1;
  handler->buf_size = 1024 * 1024 * 1024;
  handler->buf = malloc(handler->buf_size);
  handler->send_buf_size = 512 * 1024 * 1024;
  handler->receive_buf_size = 512 * 1024 * 1024;
  handler->send_buf = handler->buf;
  handler->receive_buf = handler->send_buf + handler->send_buf_size;
  handler->have_read = 0;
  handler->have_send = 0;
  handler->write_offset = 0;
  handler->is_on = true;
  handler->addr = NULL;
  handler->mr = NULL;
  reg_buffer(handler);
  create_cq_and_qp(handler, 100, IBV_QPT_RC);
  sync_qp_info(handler);
  modify_qp_to_rts_and_rtr(handler);
}
