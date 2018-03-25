#include "server_comm.h"

void SequentialServer(string coor_addr,
                      int coor_port,
                      string self_addr,
                      int self_port,
                      int socket_fd) {
  // first: reply to #, second: content
  queue<pair<int, string> > to_be_assigned_articles;
  // first: remote ip addr, second: port num
  queue<pair<string, int> > to_be_replied_list;
  // first remote ip addr, second; port num
  queue<pair<string, int> > to_be_repiled_full;

  char buf[4096];
  struct sockaddr_in si_other;
  socklen_t socketlen = sizeof(si_other);

  printf("Sequential Server <%s:%d> Launched\n", self_addr.c_str(), self_port);

  while (true) {
    // start to receive incoming messages
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
      continue;
    }

    string req = string(buf);
    if (req[0] == 'C') {
      // ping request
      string remote_ip;
      int remote_port;
      ParsePingReqPacket(req, remote_ip, remote_port);
      printf(
        "Received ping from client <%s:%d>\n",
        remote_ip.c_str(),
        remote_port
      );
      string PingACK = FormPingPacket(local_addr, local_port, 'A');
      if (
        UDP_send_packet_socket(
          PingACK.c_str(),
          remote_ip.c_str(),
          remote_port,
          socket_fd
        ) == -1) {
        printf("Error: met error in sending the packet out");
        continue;
      }
      printf("ACK reply sent\n");
    } else if (req[0] == 'P') {
      // post request (for article or reply)
      string source_ip;
      int source_port, reply_to_num;
      string article_content;
      ParsePostReqPacket(
        req,
        source_ip,
        source_port,
        reply_to_num,
        article_content
      );
      printf(
        "Received a Post request from <%s:%d>\n",
        source_ip.c_str();
        source_port
      );
      // queue this article until it got an unique number
      to_be_assigned_articles.push_back(
        make_pair(reply_to_num, article_content)
      );
      // TODO
    } else if (req[0] == 'R') {
      // read request
    } else if (req[0] == 'V') {
      // view full request
    } else if (req[0] == '0') {
      // unique id assignment reply
    } else {
      printf("Received unauthorized request symbol \"%c\"\n", req[0]);
    }
  }

  return;
}

void SequentialServerCoor(string self_addr,
                          int self_port,
                          int socket_fd) {
  return;
}
