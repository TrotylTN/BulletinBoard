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

string FormReadReqPacket(string local_addr, int local_port);
void ParseReadReqPacket(string rev_packet, string &remote_ip, int &remote_port);
