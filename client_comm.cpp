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
    perror("Timeout or recvfrom error");
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
  // cache the list of all articles, just pass the needed information
  int cache_length = 0;
  pair<int, string> list_cache[10000];
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
      // form read request for current client
      string ReadReq = FormReadReqPacket(
        local_ip_addr,
        local_port_num,
        cache_length
      );
      if (
        UDP_send_packet_socket(
          ReadReq.c_str(),
          server_ip_addr.c_str(),
          server_port_num,
          socket_fd
        ) == -1
      ) {
        // met error in sending the packet out
        printf("Error: met some error in sending request for Read\n");
        continue;
      }
      printf(
        "Read request has been sent to the server <%s:%d>\n",
        server_ip_addr.c_str(),
        server_port_num
      );

      // start to wait for response
      char buf[4096];
      struct sockaddr_in si_other;
      socklen_t socketlen = sizeof(si_other);
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
        perror("Timeout or recvfrom error");
        printf("Error: met some errors in receiving updates\n");
        continue;
      }

      string readReply = string(buf);
      int new_length, cur_num, reply_to_num;
      string first_50_abstract;
      ParseReadReplyPacket(
        readReply,
        new_length,
        cur_num,
        reply_to_num,
        first_50_abstract
      );
      if (cur_num == 0 || cache_length - new_length == 0) {
        // no need to update, continue directly
        printf("Local cache is already updated, no need to update again\n");
      } else {
        // TODO: start a loop to receive
        printf("Received %d new articles/replies, updating...\n", new_length);
        list_cache[cur_num] = make_pair(reply_to_num, first_50_abstract);
        int rest_updates = cache_length - new_length - 1;
        while (rest_updates > 0) {
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
            perror("Timeout or recvfrom error");
            printf("Error: met some errors in receiving updates\n");
            break;
          }
          readReply = string(buf);
          ParseReadReplyPacket(
            readReply,
            new_length,
            cur_num,
            reply_to_num,
            first_50_abstract
          );
          // update the cache
          list_cache[cur_num] = make_pair(reply_to_num, first_50_abstract);
          rest_updates--;
        }
      }
      PrintAllCaches(cache_length, list_cache);
    } else if (choice_num == 3) {
      // choose an article or reply to view the full content
      if (cache_length == 0) {
        printf("You must get the updated articles/replies list first\n");
        // back to the main menu
        continue;
      }
      printf("For viewing new articles/replies not in cache, ");
      printf("you must Read the list for all articles & replies first\n");
      printf("Please enter the article/reply you want to view full content ");
      printf("(Range from 1 to %d): ", cache_length);
      int article_num;
      cin >> article_num;
      // create a ViewReq packet
      string ViewReq = FormViewReqPacket(
        local_ip_addr,
        local_port_num,
        article_num
      );
      if (
        UDP_send_packet_socket(
          ViewReq.c_str(),
          server_ip_addr.c_str(),
          server_port_num,
          socket_fd
        ) == -1
      ) {
        // met error in sending the packet out
        printf("Error: met some error in sending request for View\n");
        continue;
      }
      printf(
        "View request has been sent to the server <%s:%d>\n",
        server_ip_addr.c_str(),
        server_port_num
      );

      // start to receive response for ViewReq


    } else if (choice_num == 4) {

    } else {
      printf("Wrong choice number, please try again\n");
    }
  }
}
