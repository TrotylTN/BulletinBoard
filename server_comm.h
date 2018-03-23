#include "comm.h"

// this function will return the implement the coordinator server took
// if -1, means connecting met error
int RegisterThisServer(string coor_addr,
                       int coor_port,
                       string self_addr,
                       int self_port,
                       int socket_fd);

void SequentialServer(string coor_addr,
                      int coor_port,
                      string self_addr,
                      int self_port,
                      int socket_fd);
void SequentialServerCoor(string self_addr,
                          int self_port,
                          int socket_fd);

void QuorumServer(string coor_addr,
                      int coor_port,
                      string self_addr,
                      int self_port,
                      int socket_fd);
void QuorumServerCoor(string self_addr,
                      int self_port,
                      int socket_fd);

void RAndWServer(string coor_addr,
                 int coor_port,
                 string self_addr,
                 int self_port,
                 int socket_fd);
void RAndWServerCoor(string self_addr,
                     int self_port,
                     int socket_fd);
