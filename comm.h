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

string FormPingPacket(string local_addr, int local_port, char msgstatus);
void ParsePingReqPacket(string rev_packet, string &remote_ip, int &remote_port);
bool ParsePingACKPacket(string rev_packet);

string FormPostPacket(string local_addr,
                      int local_port,
                      int reply_to_num,
                      string article_content);
void ParsePostReqPacket(string rev_packet,
                        string &remote_ip,
                        int &remote_port,
                        int &reply_to_num,
                        string &article_content);

string FormReadReqPacket(string local_addr, int local_port, int cache_length);
void ParseReadReqPacket(
  string rev_packet,
  string &remote_ip,
  int &remote_port,
  int &cache_length
);

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

string FormViewReqPacket(string local_addr, int local_port, int article_num);
void ParseViewReqPacket(
  string rev_packet,
  string &remote_ip,
  int &remote_port,
  int &article_num
);

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

string FormNumReqPacket(string local_addr, int local_port);
void ParseNumReqPacket(
  string recv_packet,
  string &remote_ip,
  int &remote_port
);

string FormNumReplyPacket(int assigned_num);
void ParseNumReplyPacket(string recv_packet, int &assigned_num);
