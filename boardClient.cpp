#include "client_comm.h"

string local_ip_addr;
int local_port_num;
int local_socket_fd;

string server_ip_addr;
int server_port_num;

int main() {
  // get local IP address in string form via self-defined API
  local_ip_addr = get_local_IP();
  printf("Which port will be used to set up this client: ");
  cin >> local_port_num;

  // initialize socket
  struct sockaddr_in si_me;
  if ((local_socket_fd=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
    perror("creating socket failed");
    return -1;
  }

  // set time out arg for the socket, timeout limit will be 5 sec
  struct timeval tv;
  tv.tv_sec = MAXTIMEOUTSEC;
  tv.tv_usec = 0;
  if (setsockopt(local_socket_fd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
      perror("Set timeout error");
  }

  // bind the socket to the specific port
  memset((char *) &si_me, 0, sizeof(si_me));
  si_me.sin_family = AF_INET;
  si_me.sin_port = htons(local_port_num);
  si_me.sin_addr.s_addr = htonl(INADDR_ANY);
  if(bind(local_socket_fd , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1) {
    perror("socket bind error");
    return -1;
  }
  printf("Local address is <%s:%d>\n", local_ip_addr.c_str(), local_port_num);
  printf("Socket has been successfully initialized for the local address\n\n");

  while (true) {
    printf("Enter the server address you want to connect: ");
    cin >> server_ip_addr;
    printf("Enter the server port you want to connect: ");
    cin >> server_port_num;

    int status_connection = PingThisServer(
      server_ip_addr,
      server_port_num,
      local_ip_addr,
      local_port_num,
      local_socket_fd
    );

    if (status_connection != -1) {
      BulletClient(
        server_ip_addr,
        server_port_num,
        local_ip_addr,
        local_port_num,
        local_socket_fd
      );
    } else {
      printf("Cannot reach the server <%s:%d>\n",
             server_ip_addr.c_str(),
             server_port_num);
      printf("Please try again\n\n");
    }
  }
  return 0;
}
