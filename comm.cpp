#include "comm.h"

// get local ip address
string get_local_IP(void) {
  struct ifaddrs *ifa=NULL, *i=NULL;
  getifaddrs(&ifa);
  string local_ip = "";
  for (i = ifa; i != NULL; i = i->ifa_next) {
    if (!i->ifa_addr)
        continue;
    if (i->ifa_addr->sa_family == AF_INET) {
      char buf[INET_ADDRSTRLEN];
      inet_ntop(
        AF_INET,
        &((struct sockaddr_in *)i->ifa_addr)->sin_addr,
        buf,
        INET_ADDRSTRLEN
      );
      if (string(buf) != "127.0.0.1") {
        local_ip = string(buf);
        break;
      }
    }
  }
  if (ifa != NULL) freeifaddrs(ifa);
  return local_ip;
}

// UDP help function implementation
int UDP_send_packet_socket(const char *packet_content,
                           const char *dest_IP,
                           const unsigned short dest_port,
                           int s) {
  struct sockaddr_in si_other;
  int slen=sizeof(si_other);

  memset((char *) &si_other, 0, sizeof(si_other));
  si_other.sin_family = AF_INET;
  si_other.sin_port = htons(dest_port);

  if (inet_aton(dest_IP , &si_other.sin_addr) == 0)
  {
      fprintf(stderr, "inet_aton() failed\n");
      return -1;
  }
  // send the message
  if (sendto(s, packet_content, strlen(packet_content), 0,
             (struct sockaddr *) &si_other, slen)==-1) {
    perror("sendto failed");
    return -1;
  }
  return 0;
}
