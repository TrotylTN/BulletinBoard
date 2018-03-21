#include "comm.h"

string local_ip_addr;
int local_port_num;
int local_socket_fd = 0;

string coordinator_server_addr = "";
int coordinator_server_port = 0;
int mode_num = 0;

int main() {
  // get local IP address in string form via self-defined API
  local_ip_addr = get_local_IP();
  printf("Which port will be used to set up this server: ");
  cin >> local_port_num;

  // initialize socket
  struct sockaddr_in si_me;
  if ((local_socket_fd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
    perror("creating socket failed");
    return -1;
  }
  memset((char *) &si_me, 0, sizeof(si_me));
  si_me.sin_family = AF_INET;
  si_me.sin_port = htons(local_port_num);
  si_me.sin_addr.s_addr = htonl(INADDR_ANY);
  if(bind(local_socket_fd , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1) {
    perror("socket bind error");
    return -1;
  }
  printf("Local address is %s:%d\n\n", local_ip_addr.c_str(), local_port_num);
  printf("Socket has been successfully initialized for the local address\n\n");

  printf("Is this server a coordinator server (Y/N): ");
  string rtoq;
  cin >> rtoq;
  if (rtoq == "Y" or rtoq == "y") {
    // this server will be a coordinator server
    printf("This server will be initialized as a coordinator server\n");
    while (true) {
      printf("Which implement you want to apply?\n");
      printf("  1.sequential consistency\n  2.quorum consistency\n");
      printf("  3.Read-your-Write ​ consistency\nPlease enter your choice: ");
      cin >> mode_num;
      if (mode_num >= 1 && mode_num <= 3) {
        printf("\nThe server will be initialized in method <%d>\n\n");
        break;
      } else {
        printf("Invalid number entered! Try again.\n\n");
      }
    }

  } else {
    // this server will be a regular server
    printf("Please enter the coordinator server's IP address: ");
    cin >> coordinator_server_addr;
    printf("Please enter the port number for the coordinator server: ");
    cin >> coordinator_server_port;
    printf("This server will be initialized as a regular server\n");

    printf("\n");

  }

}
