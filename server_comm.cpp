#include "server_comm.h"

int RegisterThisServer(string coor_addr,
                       int coor_port,
                       string self_addr,
                       int self_port,
                       int socket_fd) {
  char buf[4096];
  struct sockaddr_in si_other;
  socklen_t socketlen = sizeof(si_other);

  // send server register request
  string ServerReg = FormServerRegPacket(
    self_addr,
    self_port,
    0
  );
  if (
    UDP_send_packet_socket(
      ServerReg.c_str(),
      coor_addr.c_str(),
      coor_port,
      socket_fd
    ) == -1) {
    printf("Error: met error in sending ServerReg out");
    return -1;
  }
  // receive registration confirmation
  if (
    recvfrom(
      socket_fd,
      buf,
      4096,
      0,
      (struct sockaddr *) &si_other,
      &socketlen
    ) < 0
  ) {
    perror("timeout");
    return -1;
  }

  string req = string(buf);

  char mode_num;
  ParseServerRegPacket(
    req,
    self_addr,
    self_port,
    mode_num
  );
  if (mode_num >= '1' && mode_num <= '3') {
    return (mode_num - '0');
  }
  return -1;
}
