#include <iostream>
#include <string>
#include "comm.h"

using namespace std;

int main() {
  cout << string(0, 'c') << endl;
  string p = FormPingPacket("192.168.101.1", 8848, 'R');
  string ip_addr;
  int port_num;
  ParsePingReqPacket(p, ip_addr, port_num);
  cout << ip_addr << '|' << endl << port_num << 'l' << endl;

  string post = FormPostPacket("192.168.1.1", 5554, 855, "hello world, here is tiannan");
  cout << post << endl;
  int reply_to_num;
  string article;
  ParsePostReqPacket(post, ip_addr, port_num, reply_to_num, article);
  cout << ip_addr << "|" << port_num << '|' << reply_to_num << '|' << article << "|\n";

  string readreq = FormReadReqPacket("192.168.2.105", 2243, 554);
  cout << readreq << endl;
  int so_far_num;
  ParseReadReqPacket(readreq, ip_addr, port_num, so_far_num);
  cout << ip_addr << "|\n" << port_num << "|\n" << so_far_num << "|\n";

  string readreply = FormReadReplyPacket(52, 16, 4, "hello thisis abstract");
  cout << readreply << endl;
  int i1, i2, i3;
  ParseReadReplyPacket(readreply, i1, i2, i3, article);
  cout << i1 << ", "  << i2 << ", " << i3 << endl;
  cout << article << endl;

  string viewreply = FormViewReplyPacket(22, 0, "hello thisis the full content");
  cout << viewreply << endl;
  ParseViewReplyPacket(readreply, i2, i3, article);
  cout << i2 << ", " << i3 << endl;
  cout << article << endl;


}
