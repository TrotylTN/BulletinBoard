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

// print all caches out
void PrintAllCaches(int n, pair<int, string> list_cache[10000]) {
  // TODO
  return;
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
 * Post Sending Packet:
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
 * ReadReq Sending Packet:
 * P[0]: R
 * P[1:16]: Source IP
 * P[16:21]: Source port
 * P[21:25]: Cache limit
 * we assume all incoming args are valid
 */
string FormReadReqPacket(string local_addr, int local_port, int cache_length) {
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

  res += to_string(cache_length);
  cur_len = res.length();
  res += string(25 - cur_len, ' ');
  // so far length should be 25

  return res;
}

void ParseReadReqPacket(
  string rev_packet,
  string &remote_ip,
  int &remote_port,
  int &cache_length
) {
  // extract remote ip addr
  remote_ip = remove_all_end_spaces(rev_packet.substr(1, 15));

  // extract remote port number
  string remote_port_str = remove_all_end_spaces(rev_packet.substr(16, 5));
  remote_port = stoi (remote_port_str,nullptr);

  // extract cache length
  string cache_length_str = remove_all_end_spaces(rev_packet.substr(21, 4));
  cache_length = stoi (cache_length_str,nullptr);

  return;
}

/*
 * readReply Sending Packet:
 * P[0]: L
 * P[1:5]: the updated list length (max 9999)
 * P[5:9]: the number for the current article
 * P[9:13]: the number this reply replies to (if article here will be 0)
 * P[13:63]: first 50 characters abstract for this reply or article
 * we assume all incoming args are valid
 */
string FormReadReplyPacket(
  int new_list_length,
  int cur_num,
  int reply_to_num,
  string first_50_abstract
) {
  string res = "L";

  res += to_string(new_list_length);
  res += string(5 - res.length(), ' ');
  // should be 5 here

  res += to_string(cur_num);
  res += string(9 - res.length(), ' ');
  // should be 9

  res += to_string(reply_to_num);
  res += string(13 - res.length(), ' ');
  // should be 13

  res += first_50_abstract;

  return res;
}

void ParseReadReplyPacket(
  string recv_packet,
  int &new_list_length,
  int &cur_num,
  int &reply_to_num,
  string &first_50_abstract
) {
  string new_length_str = remove_all_end_spaces(recv_packet.substr(1, 4));
  new_list_length = stoi(new_length_str, nullptr);

  string cur_num_str = remove_all_end_spaces(recv_packet.substr(5, 4));
  cur_num = stoi(cur_num_str, nullptr);

  string reply_to_num_str = remove_all_end_spaces(recv_packet.substr(9, 4));
  reply_to_num = stoi(reply_to_num_str, nullptr);

  first_50_abstract = recv_packet.substr(13);

  return;
}

/*
 * ViewReq Sending Packet:
 * P[0]: V
 * P[1:16]: Source IP
 * P[16:21]: Source port
 * P[21:25]: Cache limit
 * we assume all incoming args are valid
 */
string FormViewReqPacket(string local_addr, int local_port, int article_num) {
  string res = "V";

  res += local_addr;
  res += string(16 - res.length(), ' ');
  // so far length should be 16

  res += to_string(local_port);
  res += string(21 - res.length(), ' ');
  // so far length should be 21

  res += to_string(article_num);
  res += string(25 - res.length(), ' ');
  // so far length should be 25

  return res;
}

void ParseViewReqPacket(
  string rev_packet,
  string &remote_ip,
  int &remote_port,
  int &article_num
) {
  // extract remote ip addr
  remote_ip = remove_all_end_spaces(rev_packet.substr(1, 15));

  // extract remote port number
  string remote_port_str = remove_all_end_spaces(rev_packet.substr(16, 5));
  remote_port = stoi (remote_port_str,nullptr);

  // extract article number
  string article_num_str = remove_all_end_spaces(rev_packet.substr(21, 4));
  article_num = stoi (article_num_str,nullptr);

  return;
}

/*
 * ViewReply Sending Packet:
 * P[0]: F
 * P[1:5]: the number for the current article
 * P[5:9]: the number this reply replies to (if article here will be 0)
 * P[9:]: the full content for the article/reply
 * we assume all incoming args are valid
 */
string FormViewReplyPacket(
  int cur_num,
  int reply_to_num,
  string full_content
) {
  string res = "F";

  res += to_string(cur_num);
  res += string(5 - res.length(), ' ');
  // should be 5 here

  res += to_string(reply_to_num);
  res += string(9 - res.length(), ' ');
  // should be 9

  res += full_content;

  return res;
}

void ParseViewReplyPacket(
  string recv_packet,
  int &cur_num,
  int &reply_to_num,
  string &full_content
) {
  string cur_num_str = remove_all_end_spaces(recv_packet.substr(1, 4));
  cur_num = stoi(cur_num_str, nullptr);

  string reply_to_num_str = remove_all_end_spaces(recv_packet.substr(5, 4));
  reply_to_num = stoi(reply_to_num_str, nullptr);

  full_content = recv_packet.substr(9);

  return;
}

/*
 * NumReq Sending Packet:
 * P[0]: N
 * P[1:16]: Source IP
 * P[16:21]: Source port
 */
string FormNumReqPacket(string local_addr, int local_port) {
  string res = "N";

  res += local_addr;
  res += string(16 - res.length(), ' ');
  // length should be 16 here

  res += to_string(local_port);
  res += string(21 - res.length(), ' ');
  // should be 21 here

  return res;
}
void ParseNumReqPacket(string recv_packet, string &remote_ip, int &remote_port){
  // extract reomte ip address
  remote_ip = remove_all_end_spaces(recv_packet.substr(1, 15));
  // extract port number
  string remote_port_str = remove_all_end_spaces(recv_packet.substr(16, 5));
  remote_port = stoi (remote_port_str,nullptr);

  return;
}

/*
 * Num Reply Sending Packet:
 * P[0]: 0
 * P[1:5]: Assigned number for the new article
 */
string FormNumReplyPacket(int assigned_num) {
  string res = "0";

  res += to_string(assigned_num);
  res += string(5 - res.length(), ' ');
  // should be 5 here
  return res;
}
void ParseNumReplyPacket(string recv_packet, int &assigned_num) {
  // extract assigned_num
  string num_str = remove_all_end_spaces(recv_packet.substr(1, 4));
  assigned_num = stoi (num_str,nullptr);
  return;
}

/*
 * QueryReq Sending Packet:
 * P[0]: Q
 * P[1:16]: local ip addr
 * P[16:21]: local port
 * P[21]: 'A' for getting all articles, 'S' for getting a specific article
 * P[22]: 'A' for abstract (first 50 char), 'F' for full content (4000 max char)
 * P[23:27]: if getting all, where to start, if a specific, which one
 * P[27:42]: client id addr
 * P[42:47]: client port number
 */
string FormQueryReqPacket(
  string local_addr,
  int local_port,
  char all_or_specific,
  char abstract_or_full_content,
  int from_which_article,
  string client_ip_addr,
  int client_port
) {
    string res = "Q";

    res += local_addr;
    res += string(16 - res.length(), ' ');
    // should be 16
    res += to_string(local_port);
    res += string(21 - res.length(), ' ');
    // should be 21
    res.push_back(all_or_specific);
    res.push_back(abstract_or_full_content);
    // should be 23 here
    res += to_string(from_which_article);
    res += string(27 - res.length(), ' ');
    // should be 27 here
    res += client_ip_addr;
    res += string(42 - res.length(), ' ');
    // should be 42
    res += to_string(client_port);
    res += string(47 - res.length(), ' ');
    // should be 47 here

    return res;
}

void ParseQueryReqPacket(
  string recv_packet,
  string &remote_ip,
  int &remote_port,
  char &all_or_specific,
  char &abstract_or_full_content,
  int &from_which_article,
  string &client_ip_addr,
  int &client_port
) {
  // 1-16
  remote_ip = remove_all_end_spaces(recv_packet.substr(1, 15));
  // 16-21
  string remote_port_str = remove_all_end_spaces(recv_packet.substr(16, 5));
  remote_port = stoi(remote_port_str, nullptr);
  // 21,22
  all_or_specific = recv_packet[21];
  abstract_or_full_content = recv_packet[22];
  // 23-27
  string article_str = remove_all_end_spaces(recv_packet.substr(23, 4));
  from_which_article = stoi(article_str, nullptr);
  // 27-42
  client_ip_addr = remove_all_end_spaces(recv_packet.substr(27, 15));
  // 42-47
  string client_port_str = remove_all_end_spaces(recv_packet.substr(42, 5));
  client_port = stoi(client_port_str, nullptr);

  return;
}


/*
 * Query Reply Sending Packet:
 * P[0]: A
 * P[1:5]: how many packets have been sent
 * P[5:9]: unique id for this packet
 * P[9:13]: this article's reply to number, if new article, here should be 0
 * P[13:28]: client ip addr
 * P[28:33]: client port number
 * P[33,37]: updated length of storage
 * P[37:]: Article content
 */
string FormQueryReplyPacket(
  int total_packet_sent,
  int unique_id_this_article,
  int reply_to_num,
  string client_ip_addr,
  int client_port,
  int updated_length,
  string full_content
) {
  string res = "A";

  res += to_string(total_packet_sent);
  res += string(5 - res.length(), ' ');
  // should be 5
  res += to_string(unique_id_this_article);
  res += string(9 - res.length(), ' ');
  // should be 9
  res += to_string(reply_to_num);
  res += string(13 - res.length(), ' ');
  // should be 13 here
  res += client_ip_addr;
  res += string(28 - res.length(), ' ');
  // should be 28 here, ip max length is 15
  res += to_string(client_port);
  res += string(33 - res.length(), ' ');
  // should be 33
  res += to_string(updated_length);
  res += string(37 - res.length(), ' ');
  res += full_content;
  // max 4037 < 4096
  return res;
}

void ParseQueryReplyPacket(
  string recv_packet,
  int &total_packet_sent,
  int &unique_id_this_article,
  int &reply_to_num,
  string &client_ip_addr,
  int &client_port,
  int &updated_length,
  string &full_content
) {
  // 1-5
  string total_packet_str = remove_all_end_spaces(recv_packet.substr(1, 4));
  total_packet_str = stoi(total_packet_str, nullptr);
  // 5-9
  string unique_id_str = remove_all_end_spaces(recv_packet.substr(5, 4));
  unique_id_this_article = stoi(unique_id_str, nullptr);
  // 9-13
  string reply_to_num_str = remove_all_end_spaces(recv_packet.substr(9, 4));
  reply_to_num = stoi(reply_to_num_str, nullptr);
  // 13-28
  client_ip_addr = remove_all_end_spaces(recv_packet.substr(13, 15));
  // 28-33
  string client_port_str = remove_all_end_spaces(recv_packet.substr(28, 5));
  client_port = stoi(client_port_str, nullptr);
  // 33-37
  string length_str = remove_all_end_spaces(recv_packet.substr(33, 4));
  updated_length = stoi(length_str, nullptr);
  // 37: max 4000
  full_content = recv_packet.substr(37);
  return;
}

/*
 * Broadcast Packet
 * P[0]: B
 * P[1:5]: article number
 * P[5:9]: reply_to_num
 * P[9:]: full content
 */
string FormBroadcastPacket(
  int unique_id,
  int reply_to_num,
  string full_content
) {
  string res = "B";

  res += to_string(unique_id);
  res += string(5 - res.length(), ' ');
  // should be 5
  res += to_string(reply_to_num);
  res += string(9 - res.length(),' ');
  // should be 9
  res += full_content;

  return res;
}

void ParseBroadcastPacket(
  string recv_packet,
  int &unique_id,
  int &reply_to_num,
  string &full_content
) {
  // 1-5
  string unique_id_str = remove_all_end_spaces(recv_packet.substr(1, 4));
  unique_id = stoi(unique_id_str, nullptr);
  // 5-9
  string reply_to_num_str = remove_all_end_spaces(recv_packet.substr(5, 4));
  reply_to_num = stoi(reply_to_num_str, nullptr);
  // 9:
  full_content = recv_packet.substr(9);

  return;
}
