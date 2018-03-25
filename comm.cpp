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

string remove_all_end_spaces(string s) {
  int slen = s.length();
  int i = slen - 1;
  while (i >= 0 && s[i] == ' ')
    i--;
  s.erase(i + 1, slen - i - 1);
  return s;
}

/*
 * Ping Sending Packet:
 * P[0]: C
 * P[1]: 'R' stands for this is a request, 'A' stands for this is an ACK
 * P[2:17]: Source IP
 * P[17:22]: Source port
 * we assume all incoming args are valid
 */
string FormPingPacket(string local_addr, int local_port, char msgstatus) {
  int cur_len;
  string res = "C";
  res.push_back(msgstatus);
  // so far length should be 2
  res += local_addr;
  cur_len = res.length();
  res += string(17 - cur_len, ' ');
  // so far length should be 17
  res += to_string(local_port);
  cur_len = res.length();
  res += string(22 - cur_len, ' ');

  // cerr << res << endl;
  return res;
}

void ParsePingReqPacket(string rev_packet, string &remote_ip, int &remote_port){
  // extract remote ip addr
  remote_ip = rev_packet.substr(2, 15);
  remote_ip = remove_all_end_spaces(remote_ip);

  // extract remote port number
  string remote_port_str = rev_packet.substr(17, 5);
  remote_port_str = remove_all_end_spaces(remote_port_str);
  remote_port = stoi (remote_port_str,nullptr);

  return;
}

bool ParsePingACKPacket(string rev_packet) {
  // if P[1] is 'A', it's an ACK
  return (rev_packet[1] == 'A');
}

/*
 * Ping Sending Packet:
 * P[0]: P
 * P[1:16]: Source IP
 * P[16:21]: Source port
 * P[21:25]: Reply-to number, if zero, means this is a new article\
 * P[25:]: article content
 * we assume all incoming args are valid
 */
string FormPostPacket(string local_addr,
                      int local_port,
                      int reply_to_num,
                      string article_content) {
  string res;
  int cur_len;
  res = "P";
  res += local_addr;
  cur_len = res.length();
  res += string(16 - cur_len, ' ');
  // so far length should be 16
  res += to_string(local_port);
  cur_len = res.length();
  res += string(21 - cur_len, ' ');
  // so far length should be 21
  res += to_string(reply_to_num);
  cur_len = res.length();
  res += string(25 - cur_len, ' ');
  // so far length should be 25
  res += article_content;
  return res;
}

void ParsePostReqPacket(string rev_packet,
                        string &remote_ip,
                        int &remote_port,
                        int &reply_to_num,
                        string &article_content) {
  // extract remote ip addr
  remote_ip = remove_all_end_spaces(rev_packet.substr(1, 15));

  // extract remote port number
  string remote_port_str = remove_all_end_spaces(rev_packet.substr(16, 5));
  remote_port = stoi (remote_port_str,nullptr);

  // extract reply_to_num
  string reply_to_num_str = remove_all_end_spaces(rev_packet.substr(21, 4));
  reply_to_num = stoi(reply_to_num_str, nullptr);

  article_content = rev_packet.substr(25);

  return;
}

/*
 * Ping Sending Packet:
 * P[0]: R
 * P[1:16]: Source IP
 * P[16:21]: Source port
 * we assume all incoming args are valid
 */
string FormReadReqPacket(string local_addr, int local_port) {
  string res;
  int cur_len;
  res = "R";

  res += local_addr;
  cur_len = res.length();
  res += string(16 - cur_len, ' ');
  // so far length should be 16

  res += to_string(local_port);
  cur_len = res.length();
  res += string(21 - cur_len, ' ');
  // so far length should be 21

  return res;
}

void ParseReadReqPacket(string rev_packet, string &remote_ip, int &remote_port){
  // extract remote ip addr
  remote_ip = remove_all_end_spaces(rev_packet.substr(1, 15));

  // extract remote port number
  string remote_port_str = remove_all_end_spaces(rev_packet.substr(16, 5));
  remote_port = stoi (remote_port_str,nullptr);

  return;
}
