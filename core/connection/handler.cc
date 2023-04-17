#include "handler.h"

int open_device_and_alloc_pd(context_info *ib_info) {
  struct ibv_device *dev = get_ib_device(0);
  ib_info->dev = dev;
  ib_info->context = ibv_open_device(dev);
  CPEN(ib_info);
  ib_info->pd = ibv_alloc_pd(ib_info->context);
  CPEN(ib_info->context);
  return 0;
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

int client_exchange(const char *server, uint16_t port) {
  int s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
  if (s == -1) {
    printf("SOCK ERROR!\n");
  }
  struct sockaddr_in sin;
  FILL(sin);
  sin.sin_family = PF_INET;
  sin.sin_port = htons(port);
  sin.sin_addr = inet_addr(server);
  m_nano_sleep(50000000);
  CPE((connect(s, (struct sockaddr *)&sin, sizeof(sin)) == -1));
  return s;
}

int server_exchange(const char *server, uint16_t port) {
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
  sin.sin_addr.s_addr = inet_addr(server);
  CPE((bind(s, (struct sockaddr *)&sin, sizeof(sin)) == -1));
  CPE((listen(s, 1) == -1));
  struct sockaddr_in csin;
  socklen_t csinsize = sizeof(csin);
  int c = accept(s, (struct sockaddr *)&csin, &csinsize);
  CPE((c == -1));
  return c;
}

int handler::build_rdma_connection() {
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

int handler::get_context_info(context_info *ib_info) {
  context = ib_info->context;
  //    handler->dev = ib_info->dev;
  pd = ib_info->pd;
  return 0;
}