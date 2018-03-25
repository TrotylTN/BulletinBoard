#include "client_comm.h"

// Try to ping the server, it will use 10 seconds to wait for server's response
// If not received any response, it will return -1. Otherwise, return 0
int PingThisServer(string server_ip_addr,
                   int server_port_num,
                   string local_ip_addr,
                   int local_port_num,
                   int socket_fd) {
  string PingReq = FormPingPacket(local_ip_addr, local_port_num, 'R');
  if (
    UDP_send_packet_socket(
      PingReq.c_str(),
      server_ip_addr.c_str(),
      server_port_num,
      socket_fd
    ) == -1) {
    // met error in sending the packet out
    return -1;
  }
  char buf[4096];
  struct sockaddr_in si_other;
  socklen_t socketlen = sizeof(si_other);
  if (recvfrom(socket_fd,
               buf,
               4096,
               0,
               (struct sockaddr *) &si_other,
               &socketlen) < 0)
  {
    perror("Timeout or Recfrom error");
    return -1;
  }
  string RecvPing = string(buf);
  if (ParsePingACKPacket(RecvPing))
    return 0;
  return -1;
}

void BulletClient(string server_ip_addr,
                  int server_port_num,
                  string local_ip_addr,
                  int local_port_num,
                  int socket_fd) {
  // Start to run the client
  printf("Successfully connected to server <%s:%d>\n",
         server_ip_addr.c_str(),
         server_port_num);
  while (true) {
    printf("\nPlease choose the opeartion you want:\n");
    printf("  1.Post an article\n  2.Read the list for all articles & replies");
    printf("\n  3.Choose an article/reply to view full content\n");
    printf("  4.Reply to an article or a reply\n");
    printf("Please enter your choice: ");
    int choice_num;
    cin >> choice_num;
    if (choice_num == 1) {
      string article_content;
      printf("Please enter your new article's content (not longer than 4000");
      printf(" characters and end with Enter): ");
      cin >> article_content;
      if (article_content.length() > 4000) {
        printf("Error: article too long\n");
        continue;
      } else {
        // reply_to_num is 0 because this is a new article
        string PostReq = FormPostPacket(
          local_ip_addr,
          local_port_num,
          0,
          article_content
        );
        if (
          UDP_send_packet_socket(
            PostReq.c_str(),
            server_ip_addr.c_str(),
            server_port_num,
            socket_fd
          ) == -1
        ) {
          // met error in sending the packet out
          printf("Error: met some error in posting new article\n");
          continue;
        }
        printf("Sent the post to the server <%s:%d>\n",
               server_ip_addr.c_str(),
               server_port_num
        );
      }
    } else if (choice_num == 2) {
    } else if (choice_num == 3) {

    } else if (choice_num == 4) {

    } else {
      printf("Wrong choice number, please try again\n");
    }
  }
}
