#include "server_comm.h"

void SequentialServer(string coor_addr,
                      int coor_port,
                      string self_addr,
                      int self_port,
                      int socket_fd) {
  bool is_primary = false;
  // first: reply to #, second: content
  queue<pair<int, string> > to_be_assigned_articles;
  // first: remote ip addr, second: port num
  vector<pair<string, int> > to_be_replied_read;
  vector<pair<string, int> > to_be_replied_view;

  // these two var are just for primary backup server's usage
  // first: reply to #, second: content
  pair<int, string> article_storage[10000];
  int storage_length = 0;

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
    if (req[0] == '1') {
      // became primary backup server;
      is_primary = true;
      printf("This server became Primary Backup Server\n");
    } else if (req[0] == 'C') {
      // ping request
      string remote_ip;
      int remote_port;
      ParsePingReqPacket(req, remote_ip, remote_port);
      printf(
        "Received ping from client <%s:%d>\n",
        remote_ip.c_str(),
        remote_port
      );
      string PingACK = FormPingPacket(self_addr, self_port, 'A');
      if (
        UDP_send_packet_socket(
          PingACK.c_str(),
          remote_ip.c_str(),
          remote_port,
          socket_fd
        ) == -1) {
        printf("Error: met error in sending the PING ACK out");
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
        source_ip.c_str(),
        source_port
      );
      // queue this article until it got an unique number
      to_be_assigned_articles.push(make_pair(reply_to_num, article_content));
      // next step is to send a packet to coordinator server for id assignment
      string NumReq = FormNumReqPacket(self_addr, self_port);
      if (
        UDP_send_packet_socket(
          NumReq.c_str(),
          coor_addr.c_str(),
          coor_port,
          socket_fd
        ) == -1) {
        printf("Error: met error in sending Num Request out");
        continue;
      }
      printf("Unique ID Assignment for this article has been sent\n");
    } else if (req[0] == 'R') {
      // read request
      string client_ip;
      int client_port;
      // where the client's current list end
      int client_cache_length;
      ParseReadReqPacket(
        req,
        client_ip,
        client_port,
        client_cache_length
      );
      // send the request to the primary backup server
      if (is_primary == true) {
        int start_position = client_cache_length;

        int total_packets = storage_length - start_position;
        if (total_packets <= 0) {
          string ReadReply = FormReadReplyPacket(
            storage_length,
            0,
            0,
            0,
            ""
          );
          if (
            UDP_send_packet_socket(
              ReadReply.c_str(),
              client_ip.c_str(),
              client_port,
              socket_fd
            ) == -1) {
            printf("Error: met error in sending Read reply out");
            continue;
          }
          // directly start next round of loop
          continue;
        }
        // exclude the packets which has not been stored
        for (int i = start_position + 1; i <= storage_length; i ++) {
          if (article_storage[i].first == 0 && article_storage[i].second == "")
            total_packets--;
        }

        for (int i = start_position + 1; i <= storage_length; i++) {
          if (article_storage[i].first == 0 && article_storage[i].second == ""){
            // ignore empty storage
          } else {
            // create the read reply packet
            string ReadReply = FormReadReplyPacket(
              storage_length,
              i,
              article_storage[i].first,
              total_packets,
              article_storage[i].second.substr(0,50)
            );
            if (
              UDP_send_packet_socket(
                ReadReply.c_str(),
                client_ip.c_str(),
                client_port,
                socket_fd
              ) == -1) {
              printf("Error: met error in sending Read reply out");
              continue;
            }
          }
        }
        printf(
          "Articles list sent for <%s:%d>\n",
          client_ip.c_str(),
          client_port
        );

      } else {
        // queue the client into to_be_replied
        to_be_replied_read.push_back(make_pair(client_ip, client_port));
        // we send query to coordinator and let it forward to backup server
        string QueryReq = FormQueryReqPacket(
          self_addr,
          self_port,
          'A',
          'A',
          client_cache_length,
          client_ip,
          client_port
        );
        if (
          UDP_send_packet_socket(
            QueryReq.c_str(),
            coor_addr.c_str(),
            coor_port,
            socket_fd
          ) == -1) {
          printf("Error: met error in sending Query Request out");
          continue;
        }
        printf(
          "Read request for <%s:%d> from No.%d sent to primary backup server\n",
          client_ip.c_str(),
          client_port,
          client_cache_length
        );
      }
    } else if (req[0] == 'V') {
      // view full request
      string client_ip;
      int client_port, article_num;
      ParseViewReqPacket(
        req,
        client_ip,
        client_port,
        article_num
      );
      if (is_primary) {
        string ViewReply = FormViewReplyPacket(
          article_num,
          article_storage[article_num].first,
          article_storage[article_num].second
        );
        if (
          UDP_send_packet_socket(
            ViewReply.c_str(),
            client_ip.c_str(),
            client_port,
            socket_fd
          ) == -1) {
          printf("Error: met error in sending View reply out");
          continue;
        }
        printf("View Reply sent to <%s:%d>\n", client_ip.c_str(), client_port);
        continue;
      } else {
        // queue the client into to_be_replied
        to_be_replied_view.push_back(make_pair(client_ip, client_port));
        // we send query to coordinator and let it forward to backup server
        string QueryReq = FormQueryReqPacket(
          self_addr,
          self_port,
          'S',
          'F',
          article_num,
          client_ip,
          client_port
        );
        if (
          UDP_send_packet_socket(
            QueryReq.c_str(),
            coor_addr.c_str(),
            coor_port,
            socket_fd
          ) == -1) {
          printf("Error: met error in sending Query Request out");
          continue;
        }
        printf(
          "View request for <%s:%d> for No.%d sent to primary backup server\n",
          client_ip.c_str(),
          client_port,
          article_num
        );
      }
    } else if (req[0] == '0') {
      // unique id assignment reply
      int reply_to_num = to_be_assigned_articles.front().first;
      string article_content = to_be_assigned_articles.front().second;
      int assigned_num;
      ParseNumReplyPacket(req, assigned_num);
      if (is_primary) {
        // store locally
        if (storage_length >= 10000 || assigned_num >= 10000) {
          printf("Storage is full...\n");
          continue;
        }
        storage_length = max(storage_length, assigned_num);
        article_storage[assigned_num] = make_pair(reply_to_num,article_content);
        printf("Stored No.%d: %s\n", assigned_num, article_content.c_str());
        to_be_assigned_articles.pop();
        // directly enter next loop
        continue;
      }
      // push this update to the back up server via coordinator
      string broadcastpkt = FormBroadcastPacket(
        assigned_num,
        reply_to_num,
        article_content
      );
      if (
        UDP_send_packet_socket(
          broadcastpkt.c_str(),
          coor_addr.c_str(),
          coor_port,
          socket_fd
        ) == -1) {
        printf("Error: met error in sending Query Request out");
        continue;
      }
      printf(
        "Update for No.%d sent to primary backup server\n",
        assigned_num
      );

      // pop the non-assigned article out
      to_be_assigned_articles.pop();
    } else if (req[0] == 'A') {
      // received a reply for a client's read/view request
      int total_packets, unique_id, reply_to_num;
      string client_ip;
      int client_port, updated_length;
      char request_type;
      string full_content;
      ParseQueryReplyPacket(
        req,
        total_packets,
        unique_id,
        reply_to_num,
        client_ip,
        client_port,
        updated_length,
        request_type,
        full_content
      );
      if (request_type == 'V') {
        // TODO
        // this is a reply for View
        string ViewReply = FormViewReplyPacket(
          unique_id,
          reply_to_num,
          full_content
        );
        if (
          UDP_send_packet_socket(
            ViewReply.c_str(),
            client_ip.c_str(),
            client_port,
            socket_fd
          ) == -1) {
          printf("Error: met error in sending View Reply out");
          continue;
        }
        printf(
          "No.%d sent to <%s:%d>\n",
          unique_id,
          client_ip.c_str(),
          client_port
        );
      } else {
        // this is replies for Read
        if (total_packets == 0) {
          // no need for update
          string emptyReadReply = FormReadReplyPacket(
            updated_length,
            0,
            0,
            0,
            ""
          );
          if (
            UDP_send_packet_socket(
              emptyReadReply.c_str(),
              client_ip.c_str(),
              client_port,
              socket_fd
            ) == -1) {
            printf("Error: met error in sending Read Reply out");
            continue;
          }
          printf(
            "No need for update for client <%s:%d>, an empty reply sent\n",
            client_ip.c_str(),
            client_port
          );
          continue;
        }
        // forward the first packet to client
        string aReadReply = FormReadReplyPacket(
          updated_length,
          unique_id,
          reply_to_num,
          total_packets,
          full_content
        );
        if (
          UDP_send_packet_socket(
            aReadReply.c_str(),
            client_ip.c_str(),
            client_port,
            socket_fd
          ) == -1) {
          printf("Error: met error in sending Read Reply out");
          continue;
        }
        int rest_packets = total_packets - 1;
        // continue to receive packets, and do forwarding
        while (rest_packets > 0) {
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
          if (req[0] != 'A') {
            printf("Error: Received unexpected message during forward\n");
            return;
          }
          ParseQueryReplyPacket(
            req,
            total_packets,
            unique_id,
            reply_to_num,
            client_ip,
            client_port,
            updated_length,
            request_type,
            full_content
          );
          string aReadReply = FormReadReplyPacket(
            updated_length,
            unique_id,
            reply_to_num,
            total_packets,
            full_content
          );
          if (
            UDP_send_packet_socket(
              aReadReply.c_str(),
              client_ip.c_str(),
              client_port,
              socket_fd
            ) == -1) {
            printf("Error: met error in sending Read Reply out");
            continue;
          }
          rest_packets --;
        }
        printf(
          "Forwarded all updated to <%s:%d>\n",
          client_ip.c_str(),
          client_port
        );
      }
    } else if (req[0] == 'Q' && is_primary == true) {
      // this is a primary server and received a query request
      string remote_ip;
      int remote_port;
      char is_get_all, is_full_content;
      int start_position;
      string client_ip;
      int client_port;
      ParseQueryReqPacket(
        req,
        remote_ip,
        remote_port,
        is_get_all,
        is_full_content,
        start_position,
        client_ip,
        client_port
      );
      if (is_get_all == 'A') {
        // get all article
        int total_packets = storage_length - start_position;
        // exclude the packets which has not been stored
        for (int i = start_position + 1; i <= storage_length; i ++) {
          if (article_storage[i].first == 0 && article_storage[i].second == "")
            total_packets--;
        }
        if (total_packets <= 0) {
          // no need for update
          string QueryReply = FormQueryReplyPacket (
            0,
            0,
            0,
            client_ip,
            client_port,
            storage_length,
            'R',      // this is a read-all request
            ""
          );
          if (
            UDP_send_packet_socket(
              QueryReply.c_str(),
              remote_ip.c_str(),
              remote_port,
              socket_fd
            ) == -1) {
            printf("Error: met error in sending Query Reply out");
            continue;
          }
          printf(
            "<%s:%d> no need for update, an empty packet sent\n",
            client_ip.c_str(),
            client_port
          );
          // directly enter next iteration
          continue;
        }
        // start to send packets to the requester server
        for (int i = start_position + 1; i <= storage_length; i ++) {
          if (article_storage[i].first == 0 && article_storage[i].second == ""){
            // ignore the uncached one
          } else {
            string sent_article = article_storage[i].second;
            if (is_full_content == 'A') {
              sent_article = sent_article.substr(0, 50);
            }
            string QueryReply = FormQueryReplyPacket (
              total_packets,
              i,
              article_storage[i].first,
              client_ip,
              client_port,
              storage_length,
              'R',      // this is a read-all request
              sent_article
            );
            // send this packet to the connected server
            if (
              UDP_send_packet_socket(
                QueryReply.c_str(),
                remote_ip.c_str(),
                remote_port,
                socket_fd
              ) == -1) {
              printf("Error: met error in sending Query Reply out");
              continue;
            }
          }
        }
        printf(
          "Total %d articles have been sent to <%s:%d>\n",
          total_packets,
          remote_ip.c_str(),
          remote_port
        );
      } else {
        // get a specific article
        string sent_article = article_storage[start_position].second;
        if (is_full_content == 'A') {
          sent_article = sent_article.substr(0, 50);
        }
        string QueryReply = FormQueryReplyPacket (
          1,
          start_position,
          article_storage[start_position].first,
          client_ip,
          client_port,
          storage_length,
          'V',    // this is a specific article
          sent_article
        );
        // send this packet to the connected server
        if (
          UDP_send_packet_socket(
            QueryReply.c_str(),
            remote_ip.c_str(),
            remote_port,
            socket_fd
          ) == -1) {
          printf("Error: met error in sending Query Reply out");
          continue;
        }
        printf(
          "No.%d article has been sent to <%s:%d>\n",
          start_position,
          remote_ip.c_str(),
          remote_port
        );
      }
    } else if (req[0] == 'B' && is_primary == true) {
      // received an update as a primary server
      int unique_id, reply_to_num;
      string article_content;
      ParseBroadcastPacket(
        req,
        unique_id,
        reply_to_num,
        article_content
      );
      if (storage_length >= 10000 || unique_id >= 10000) {
        printf("Storage is full...\n");
        continue;
      }
      storage_length = max(storage_length, unique_id);
      article_storage[unique_id] = make_pair(reply_to_num, article_content);
      printf("Stored No.%d: %s\n", unique_id, article_content.c_str());
    } else {
      printf("Received unauthorized request symbol \"%c\"\n", req[0]);
    }
  }

  return;
}

void SequentialServerCoor(string self_addr,
                          int self_port,
                          int socket_fd) {
  // stored important information
  char coor_mode_number = '1';
  // init total_article
  int total_article = 0;
  string primary_ip = "";
  int primary_port = -1;
  vector< pair<string,int> > server_list;

  char buf[4096];
  struct sockaddr_in si_other;
  socklen_t socketlen = sizeof(si_other);

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

    if (req[0] == 'S') {
      // receive registration request from remote server
      string remote_ip;
      int remote_port;
      char mode_num_no_use;
      ParseServerRegPacket(
        req,
        remote_ip,
        remote_port,
        mode_num_no_use
      );
      // insert this server into the server list
      server_list.push_back(make_pair(remote_ip, remote_port));
      printf("<%s:%d> connected\n", remote_ip.c_str(), remote_port);
      string ReplyMessage = FormServerRegPacket(
        remote_ip,
        remote_port,
        coor_mode_number
      );
      // return the mode number back to the remote server
      if (
        UDP_send_packet_socket(
          ReplyMessage.c_str(),
          remote_ip.c_str(),
          remote_port,
          socket_fd
        ) == -1) {
        printf("Error: met error in sending ServerReg out");
        continue;
      }
      if (primary_port == -1) {
        // this is the first server, set it to backup server
        // give the server some minutes to start
        usleep(1000);
        // grant this server primary position
        string PrimaryAccess = FormPrimaryAccessPacket(remote_ip, remote_port);
        if (
          UDP_send_packet_socket(
            PrimaryAccess.c_str(),
            remote_ip.c_str(),
            remote_port,
            socket_fd
          ) == -1) {
          printf("Error: met error in sending PrimaryAccess out");
          continue;
        }
        primary_ip = remote_ip;
        primary_port = remote_port;
        printf(
          "<%s:%d> became primary server\n",
          remote_ip.c_str(),
          remote_port
        );
      }
    } else if (req[0] == 'N') {
      // receive unique id request
      if (total_article < 9999) {
        string remote_ip;
        int remote_port;
        ParseNumReqPacket(
          req,
          remote_ip,
          remote_port
        );

        total_article ++;

        string NumReply = FormNumReplyPacket(total_article);
        if (
          UDP_send_packet_socket(
            NumReply.c_str(),
            remote_ip.c_str(),
            remote_port,
            socket_fd
          ) == -1) {
          printf("Error: met error in sending NumReply out");
          continue;
        }
        printf(
          "Grant ID %d to <%s:%d>\n",
          total_article,
          remote_ip.c_str(),
          remote_port
        );
      } else {
        // ignore this request, let server cannot process it
        printf("Error: articles number is over the limit 9999\n");
      }
    } else if (req[0] == 'B') {
      // forward Broadcast request to primary server

      // forward it to primary back up server
      if (
        UDP_send_packet_socket(
          req.c_str(),
          primary_ip.c_str(),
          primary_port,
          socket_fd
        ) == -1) {
        printf("Error: met error in sending NumReply out");
        continue;
      }
    } else if (req[0] == 'Q') {
      // forward Query request to primary server
      if (
        UDP_send_packet_socket(
          req.c_str(),
          primary_ip.c_str(),
          primary_port,
          socket_fd
        ) == -1) {
        printf("Error: met error in sending NumReply out");
        continue;
      }
    } else {
      printf("Received unauthorized request symbol \"%c\"\n", req[0]);
    }
  }
}
