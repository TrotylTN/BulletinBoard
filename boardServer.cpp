#include "comm.h"

string local_ip_addr;
int local_port_num;

int main() {
  // get local IP address in string form via self-defined API
  local_ip_addr = get_local_IP();
  printf("Which port will be used to set up this server: \n");
  cin >>
  printf("Is this server a coordinator server (Y/N): \n");
  string rtoq;
  cin >> rtoq;
  if (rtoq == "Y" or rtoq == "y") {
    // this server will be a coordinator server
    printf("This server will be initialed as a coordinator server\n");

  } else {
    // this server will be a regular server
  }

}
