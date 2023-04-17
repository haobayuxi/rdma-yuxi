#include "handler.h"

void Handler::set_fd(int fd_) { fd = fd_; }

static inline int Handler::poll_send_cq() {
  struct ibv_wc wc;
  // printf("handler addr: %p, handler->send_cq addr: %p\n", handler,
  // handler->send_cq);
  while (ibv_poll_cq(send_cq, 1, &wc) < 1)
    ;
  if (wc.status != IBV_WC_SUCCESS) {
    printf("Status: %d\n", wc.status);
    printf("Ibv_poll_cq error!\n");
    printf("Error: %s\n", strerror(errno));
    return -1;
  }
  printf("poll cq success！\n");
  return 0;
}

static inline void Handler::post_write(size_t size, size_t offset) {
  struct ibv_sge sge = {(uint64_t)buf + offset, (uint32_t)size, mr->lkey};
  struct ibv_send_wr send_wr;
  FILL(send_wr);
  send_wr.wr_id = 1;
  send_wr.next = NULL;
  send_wr.sg_list = &sge;
  send_wr.num_sge = 1;
  send_wr.imm_data = 0;
  send_wr.opcode = IBV_WR_RDMA_WRITE_WITH_IMM;
  send_wr.wr.rdma.remote_addr =
      r_private_data->buffer_addr + send_buf_size + offset;
  send_wr.wr.rdma.rkey = r_private_data->buffer_rkey;
  send_wr.send_flags = IBV_SEND_SIGNALED;

  struct ibv_send_wr *bad_send_wr;
  CPE(ibv_post_send(qp, &send_wr, &bad_send_wr));
}

void Handler::write_with_imm(char *buf, size_t size) {
  memcpy(buf + write_offset, buf, size);
  post_write(handler, size, have_send);

  auto ret = poll_send_cq(handler);
  printf("%d \n", ret);
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

int open_device_and_alloc_pd(context_info *ib_info) {
  struct ibv_device *dev = get_ib_device(0);
  ib_info->dev = dev;
  ib_info->context = ibv_open_device(dev);
  CPEN(ib_info);
  ib_info->pd = ibv_alloc_pd(ib_info->context);
  CPEN(ib_info->context);
  return 0;
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

static int get_lid(rdma_fd *handler) {
  struct ibv_port_attr port_attr;
  CPEN(handler->context);
  CPE(ibv_query_port(handler->context, handler->ib_port_base, &port_attr));
  return port_attr.lid;
}

void Handler::sync_qp_info() {
  srand(time(NULL));
  l_qp_info = (exchange_info *)malloc(sizeof(struct exchange_info));
  l_qp_info->psn = lrand48() & 0xffffff;
  l_qp_info->qpn = qp->qp_num;
  l_qp_info->lid = get_lid(handler);
  l_private_data = (private_data *)malloc(sizeof(private_data));
  l_private_data->buffer_addr = (uint64_t)buf;
  l_private_data->buffer_rkey = mr->rkey;
  l_private_data->buffer_length = mr->length;
  r_qp_info = (exchange_info *)malloc(sizeof(exchange_info));
  r_private_data = (private_data *)malloc(sizeof(private_data));

  //   printf("Local LID = %d, QPN = %d, PSN = %d\n", l_qp_info->lid,
  //   l_qp_info->qpn,
  //          l_qp_info->psn);
  //   printf("Local Addr = %ld, RKey = %d, LEN = %zu\n",
  //          l_private_data->buffer_addr, l_private_data->buffer_rkey,
  //          l_private_data->buffer_length);

  //   exchange
  ssize_t ret;
  ret = sendData1(fd, l_qp_info, sizeof(exchange_info));
  ret = readUntil1(fd, r_qp_info, sizeof(exchange_info));
  //	printf("remote lid %d, qpn %d\n",   r_qp_info->lid,
  //   r_qp_info->qpn);
  ret = sendData1(fd, l_private_data, sizeof(private_data));
  ret = readUntil1(fd, r_private_data, sizeof(private_data));
  //	printf("remote addr %ld, rkey %d\n",
  //   r_private_data->buffer_addr,
  //   r_private_data->buffer_rkey);
  printf("Remote LID = %d, QPN = %d, PSN = %d\n", r_qp_info->lid,
         r_qp_info->qpn, r_qp_info->psn);

  printf("Remote Addr = %ld, RKey = %d, LEN = %zu\n",
         r_private_data->buffer_addr, r_private_data->buffer_rkey,
         r_private_data->buffer_length);
}

static void modify_qp_to_rts_and_rtr(rdma_fd *handler) {
  struct ibv_qp_attr qp_attr;
  FILL(qp_attr);
  int flags;
  qp_attr.qp_state = IBV_QPS_RTR;
  if (mode == M_UD) {
    flags = IBV_QP_STATE;
  } else {
    flags = IBV_QP_STATE | IBV_QP_AV | IBV_QP_PATH_MTU | IBV_QP_DEST_QPN |
            IBV_QP_RQ_PSN;
    qp_attr.path_mtu = IBV_MTU_1024;
    qp_attr.dest_qp_num = r_qp_info->qpn;
    qp_attr.rq_psn = r_qp_info->psn;
    if (mode == M_RC) {
      qp_attr.max_dest_rd_atomic = 1;
      qp_attr.min_rnr_timer = 12;
      flags |= IBV_QP_MAX_DEST_RD_ATOMIC | IBV_QP_MIN_RNR_TIMER;
    }
    qp_attr.ah_attr.is_global = 0;
    qp_attr.ah_attr.dlid = r_qp_info->lid;
    qp_attr.ah_attr.sl = 0;
    qp_attr.ah_attr.src_path_bits = 0;
    qp_attr.ah_attr.port_num = ib_port_base;
  }
  printf("IB port: %d\n", ib_port_base);
  CPE(ibv_modify_qp(qp, &qp_attr, flags));

  qp_attr.qp_state = IBV_QPS_RTS;
  flags = IBV_QP_STATE | IBV_QP_SQ_PSN;
  if (mode == M_UD) {
    qp_attr.sq_psn = lrand48() & 0xffffff;
  } else {
    qp_attr.sq_psn = l_qp_info->psn;
    if (mode == M_RC) {
      qp_attr.timeout = 14;
      qp_attr.retry_cnt = 7;
      qp_attr.rnr_retry = 7;
      qp_attr.max_rd_atomic = 1;
      flags |= IBV_QP_TIMEOUT | IBV_QP_RETRY_CNT | IBV_QP_RNR_RETRY |
               IBV_QP_MAX_QP_RD_ATOMIC;
    }
  }
  CPE(ibv_modify_qp(qp, &qp_attr, flags));
}

int Handler::build_rdma_connection() {
  mode = M_RC;
  ib_port_base = 1;
  buf_size = 1024 * 1024 * 1024;
  buf = malloc(buf_size);
  send_buf_size = 512 * 1024 * 1024;
  receive_buf_size = 512 * 1024 * 1024;
  send_buf = buf;
  receive_buf = send_buf + send_buf_size;
  have_read = 0;
  have_send = 0;
  write_offset = 0;
  is_on = true;
  addr = NULL;
  mr = NULL;
  reg_buffer(handler);
  create_cq_and_qp(handler, 100, IBV_QPT_RC);
  sync_qp_info(handler);
  modify_qp_to_rts_and_rtr(handler);
}

int Handler::get_context_info(context_info *ib_info) {
  context = ib_info->context;
  //      dev = ib_info->dev;
  pd = ib_info->pd;
  return 0;
}

void Handler::init_qp() {
  struct ibv_qp_attr qp_attr;
  FILL(qp_attr);
  qp_attr.qp_state = IBV_QPS_INIT;
  qp_attr.port_num = ib_port_base;
  qp_attr.pkey_index = 0;
  if (mode == M_UD) {
    qp_attr.qkey = qkey;
  } else {
    qp_attr.qp_access_flags = IBV_ACCESS_LOCAL_WRITE | IBV_ACCESS_REMOTE_WRITE |
                              IBV_ACCESS_REMOTE_READ | IBV_ACCESS_REMOTE_ATOMIC;
  }
  int flags = IBV_QP_STATE | IBV_QP_PKEY_INDEX | IBV_QP_PORT;
  if (mode == M_UD) {
    flags |= IBV_QP_QKEY;
  } else {
    flags |= IBV_QP_ACCESS_FLAGS;
  }
  //	printf("come here!\n");
  CPE(ibv_modify_qp(qp, &qp_attr, flags));
  //	printf("QPNum = %d\n",   qp->qp_num);
}

void Handler::create_cq_and_qp(int max_depth, enum ibv_qp_type qp_type) {
  send_cq = NULL;
  recv_cq = NULL;
  qp = NULL;

  send_cq = ibv_create_cq(context, max_depth, NULL, NULL, 0);
  CPEN(send_cq);
  recv_cq = ibv_create_cq(context, max_depth, NULL, NULL, 0);
  CPEN(recv_cq);

  struct ibv_qp_init_attr qp_init_attr;
  FILL(qp_init_attr);
  qp_init_attr.send_cq = send_cq;
  qp_init_attr.recv_cq = recv_cq;
  qp_init_attr.cap.max_send_wr = max_depth;
  qp_init_attr.cap.max_recv_wr = max_depth;

  qp_init_attr.cap.max_send_sge = 1;
  qp_init_attr.cap.max_recv_sge = 1;
  qp_init_attr.cap.max_inline_data = 64;
  qp_init_attr.qp_type = qp_type;
  qp_init_attr.sq_sig_all = 0;
  qp = ibv_create_qp(pd, &qp_init_attr);
  CPEN(qp);
  init_qp(handler);
}

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
  if (listen(_socket, GROUP_SIZE)) return -1;
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