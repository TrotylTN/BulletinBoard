#include "client_comm.h"

// Try to ping the server, it will use 10 seconds to wait for server's response
// If not received any response, it will return -1. Otherwise, return 0
int PingThisServer(string server_ip_addr,
                   int server_port_num,
                   string local_ip_addr,
                   int local_port_num,
                   int socket_fd) {
  string PingReq = FormPingPacket(local_ip_addr, local_port_num, 'R');
  if (
    UDP_send_packet_socket(
      PingReq.c_str(),
      server_ip_addr.c_str(),
      server_port_num,
      socket_fd
    ) == -1) {
    // met error in sending the packet out
    return -1;
  }
  char buf[4096];
  struct sockaddr_in si_other;
  socklen_t socketlen = sizeof(si_other);
  if (recvfrom(socket_fd,
               buf,
               4096,
               0,
               (struct sockaddr *) &si_other,
               &socketlen) < 0)
  {
    perror("Timeout or Recfrom error");
    return -1;
  }
  string RecvPing = string(buf);
  if (ParsePingACKPacket(RecvPing))
    return 0;
  return -1;
}

void BulletClient(string server_ip_addr,
                  int server_port_num,
                  string local_ip_addr,
                  int local_port_num,
                  int socket_fd) {
  // Start to run the client

}
