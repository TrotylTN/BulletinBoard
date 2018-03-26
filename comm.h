#include <stdio.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <net/if.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <errno.h>
#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <queue>
#include <utility>

#define MAXCLIENT 100
#define MAXSERVER 100
#define MAXTIMEOUTSEC 5

using namespace std;

string get_local_IP(void);

int UDP_send_packet_socket(const char *packet_content,
                           const char *dest_IP,
                           const unsigned short dest_port,
                           int s);

string remove_all_end_spaces(string s);

// print all caches out
void PrintAllCaches(int n, pair<int, string> list_cache[10000]);

/*
 * Ping Sending Packet:
 * P[0]: C
 * P[1]: 'R' stands for this is a request, 'A' stands for this is an ACK
 * P[2:17]: Source IP
 * P[17:22]: Source port
 * we assume all incoming args are valid
 */
string FormPingPacket(string local_addr, int local_port, char msgstatus);
void ParsePingReqPacket(string rev_packet, string &remote_ip, int &remote_port);
bool ParsePingACKPacket(string rev_packet);

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
                      string article_content);
void ParsePostReqPacket(string rev_packet,
                        string &remote_ip,
                        int &remote_port,
                        int &reply_to_num,
                        string &article_content);

/*
 * ReadReq Sending Packet:
 * P[0]: R
 * P[1:16]: Source IP
 * P[16:21]: Source port
 * P[21:25]: Cache limit
 * we assume all incoming args are valid
 */
string FormReadReqPacket(string local_addr, int local_port, int cache_length);
void ParseReadReqPacket(
  string rev_packet,
  string &remote_ip,
  int &remote_port,
  int &cache_length
);

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
);
void ParseReadReplyPacket(
  string recv_packet,
  int &new_list_length,
  int &cur_num,
  int &reply_to_num,
  string &first_50_abstract
);

/*
 * ViewReq Sending Packet:
 * P[0]: V
 * P[1:16]: Source IP
 * P[16:21]: Source port
 * P[21:25]: Cache limit
 * we assume all incoming args are valid
 */
string FormViewReqPacket(string local_addr, int local_port, int article_num);
void ParseViewReqPacket(
  string rev_packet,
  string &remote_ip,
  int &remote_port,
  int &article_num
);

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
);
void ParseViewReplyPacket(
  string recv_packet,
  int &cur_num,
  int &reply_to_num,
  string &full_content
);

/*
 * NumReq Sending Packet:
 * P[0]: N
 * P[1:16]: Source IP
 * P[16:21]: Source port
 */
string FormNumReqPacket(string local_addr, int local_port);
void ParseNumReqPacket(
  string recv_packet,
  string &remote_ip,
  int &remote_port
);

/*
 * Num Reply Sending Packet:
 * P[0]: 0
 * P[1:5]: Assigned number for the new article
 */
string FormNumReplyPacket(int assigned_num);
void ParseNumReplyPacket(string recv_packet, int &assigned_num);

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
);
void ParseQueryReqPacket(
  string recv_packet,
  string &remote_ip,
  int &remote_port,
  char &all_or_specific,
  char &abstract_or_full_content,
  int &from_which_article,
  string &client_ip_addr,
  int &client_port
);

/*
 * QueryReq Sending Packet:
 * P[0]: A
 * P[1:5]: how many packets have been sent
 * P[5:9]: unique id for this packet
 * P[9:13]: this article's reply to number, if new article, here should be 0
 * P[13:28]: client ip addr
 * P[28:33]: client port number
 * P[33:]: Article content
 */
string FormQueryReplyPacket(
  int total_packet_sent,
  int unique_id_this_article,
  int reply_to_num,
  string client_ip_addr,
  int client_port,
  string full_content
);
void ParseQueryReplyPacket(
  string recv_packet,
  int &total_packet_sent,
  int &unique_id_this_article,
  int &reply_to_num,
  string &client_ip_addr,
  int &client_port,
  string &full_content
);

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
);

void ParseBroadcastPacket(
  string recv_packet,
  int &unique_id,
  int &reply_to_num,
  string &full_content
);
