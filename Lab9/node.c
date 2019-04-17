#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/select.h>
#include <dirent.h>

/*Server process is running on this port no. Client has to send data to this port no*/
#define SERVER_PORT		2002
#define SERVER_IP_ADDRESS	"127.0.0.1"
#define FILENAME		"file.txt"

#define NODE_NAME	"my_node"
#define NODE_IP		"127.0.0.1"
#define NODE_PORT	2001

#define h_addr h_addr_list[0]

char dbuf[1024];
char file[1024] = FILENAME;

int word_count(char* file)
{
  FILE *f = fopen(file, "r");

  if (f == NULL) return -1;
  
  char c;
  int count = 0;
  
  while ((c = fgetc(f)) != EOF)
  {
    if (c == ' ' || c == '\n')
    {
      count++;
    }
  }

  fclose(f);
  return count;
}

char** ftoa(char* file)
{
  int size = word_count(file);
  FILE *f = fopen(file, "r");
  char **data = (char **)calloc(size, sizeof(char *));

  for (int i=0; i<size; i++)  data[i] = (char *)calloc(30, sizeof(int));
  if (f == NULL) return (char **)1;
  
  char c;
  int count = 0;
  int i = -1;
  while((c = fgetc(f)) != EOF)
  {
    if(c == ' ' || c == '\n')
    {
      data[count][++i] = '\0';
      count++;
      i = -1;
    }
    else 
      data[count][++i] = c;
  }

  fclose(f);
  return data;
}

void server_add_peer_to_database(char *new_peer_name, char* ip, int port)
{
  FILE *f = fopen("peer_database.txt", "a");
  char int_string[10];

  strcat(new_peer_name, ":");
  strcat(new_peer_name, ip);
  snprintf(int_string, 10, "%d", port);
  strcat(new_peer_name, ":");
  strcat(new_peer_name, int_string);
  strcat(new_peer_name, "\n");
  printf("DB entry: %s\n", new_peer_name);

  fputs(new_peer_name, f);
  fclose(f);
}


void client_request_file(int sockfd, char *file)
{
  int sent_recv_bytes = 0;
  // SEND FILENAME TO SERVER
  char send_msg[1024];
  strcpy(send_msg, file);
  sent_recv_bytes = send(sockfd, &send_msg, sizeof(send_msg), 0);
  // printf("No of bytes sent = %d\n", sent_recv_bytes);
  // GET THE ANSWER FROM SERVER IF IT HAS FILE & # of words in file
  char result2[4];
  sent_recv_bytes =  recv(sockfd, (char *)&result2, sizeof(char)*10, 0);
  // printf("No of bytes received = %d: %s\n", sent_recv_bytes, result2);
  
  int nwords;
  memcpy(&nwords, result2, 4);

  int words_count = ntohs(nwords);

  {
    printf("file exist, request for download, #ofWords: %d\n", words_count);
    FILE *f = fopen(file, "w");

    if (f == NULL)
    { 
      printf("Unable to create file");
      exit(-1);
    }
    
    int i;
    char result3[1024] = {0};
    // REQUEST i'th WORD FROM FILE FROM SERVER
    for (i = 1; i <= words_count; i++)
    {
      // CONVERT i TO STRING msg
      snprintf(send_msg, 3, "%d", i);    
      
      // RECEIVE THE i'th WORD ROM SERVER
      // -! size of receive buffer should be greater than sent buffer of server
      
      sent_recv_bytes =  recv(sockfd, (char *)&result3, sizeof(result3), 0);
      printf("client got: %s\n", result3);

      if (i > 1) fputs(" ", f);
      fputs(result3, f);
    }
    fclose(f);
  } 
}

int server_receive_option(int comm_socket_fd)
{
  int sent_recv_bytes = 0;
  char option = -1;
  sent_recv_bytes = recv(comm_socket_fd, (char *)option, sizeof(option), 0);
  
  printf("Server got: %d\n", option);
  
  if(option == 0)
  {
    printf("will send file\n");
    return 0;
  }
  else if (option == 1)
  {
    printf("will synchronise\n");
    return 1;
  } 
  else
  {
    return -1;
    printf("Wrong option. connection refused\n");
  } 
}

char client_choose_option()
{
  char option = -1;
  
  printf("Synchronise or download file?(1|0)\n");
  scanf("%d", &option);
  
  if (option == 1)
    return 1;
  else if (option == 0)
    return 0;
  else
    return -1;
}

void client_send_option(int sockfd, char option)
{
  int sent_recv_bytes = 0;

  printf("Client ---------> server: %d\n", option);
  
  sent_recv_bytes = send(sockfd, &option, sizeof(option), 0);
  printf("No of bytes sent = %d\n", sent_recv_bytes);
}

void server_send_file(int comm_socket_fd)
{
  int sent_recv_bytes = 0;
  sent_recv_bytes = recv(comm_socket_fd, (char *)dbuf, sizeof(dbuf), 0);
  
  char result[10] = "no";
  printf("Server got: %s\n", dbuf);
  int fd = open(dbuf, 2);
  if (fd > 0)
  {
    strcpy(result, "yes ");
    int words_count = word_count(dbuf);
    char words_count_str[3] = "str";
    snprintf(words_count_str, 3, "%d", words_count);
    strcat(result, words_count_str);
  }        
  // SEND RESPONSE WITH yes/no & # of Words IN FILE
  words_count = htons(words_count);
  sent_recv_bytes = send(comm_socket_fd, (char *)&words_count, sizeof(words_count), 0);
  printf("Server ---------> client: '%s'\n", result);
  words_count = ntohs(words_count);
  char **data = ftoa(dbuf);
  int i;
  for (i = 0; i < words_count; i++)
  {
    // SEND CURRENT FILE WORD TO CLIENT
    char current_word[1024];
    strcpy(current_word, data[i-1]);
    sent_recv_bytes = send(comm_socket_fd, (char *)&current_word, sizeof(current_word), 0);
    printf("Server -----------> client: %s\n", current_word);
  }
}

void setup_tcp_server_communication()
{
  int master_sock_tcp_fd = 0, 
       sent_recv_bytes = 0, 
       addr_len = 0, 
       opt = 1;
  int comm_socket_fd = 0;
  fd_set readfds;             
  struct sockaddr_in server_addr, 
                     client_addr;
   
  if ((master_sock_tcp_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP )) == -1)
  {
    printf("socket creation failed\n");
    exit(1);
  }

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = SERVER_PORT;
  server_addr.sin_addr.s_addr = INADDR_ANY; 
  addr_len = sizeof(struct sockaddr);
   
  if (bind(master_sock_tcp_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
  {
    printf("socket bind failed\n");
    return;
  }

  struct sockaddr_in sin;
  socklen_t len = sizeof(sin);
  if (getsockname(master_sock_tcp_fd, (struct sockaddr *)&sin, &len) == -1)
      perror("getsockname");
  else
      printf("port number %d\n", ntohs(sin.sin_port));

  if (listen(master_sock_tcp_fd, 5)<0) 
  {
    printf("listen failed\n");
    return;
  }
  
  while(1)
  {
    FD_ZERO(&readfds);                     
    FD_SET(master_sock_tcp_fd, &readfds);  
    printf("blocked on select System call...\n");
    select(master_sock_tcp_fd + 1, &readfds, NULL, NULL, NULL); 
    if (FD_ISSET(master_sock_tcp_fd, &readfds))
    { 
      // Connection run
      printf("New connection recieved recvd, accept the connection. Client and Server completes TCP-3 way handshake at this point\n");
      comm_socket_fd = accept(master_sock_tcp_fd, (struct sockaddr *)&client_addr, &addr_len);
      if (comm_socket_fd < 0)
      {
	printf("accept error : errno = %d\n", errno); 
	exit(0); 
      }        
      // SERVER ESTABLISHED CONNECTION WITH NEW NODE
      printf("connection accepted\n");
      while(1)
      {
        char option;
        option = server_receive_option(comm_socket_fd);
      
        if (option == 1)
        {
	  char buff[1024] = {0};
          char new_peer_name[256] = {0}, ip[16];
	  int port;

          sent_recv_bytes = recv(comm_socket_fd, (char *)buff, sizeof(buff), 0);
	  sscanf(buff, "%[^:]:%[^:]:%d:%s", new_peer_name, ip, port, buff);
          printf("Got new node! %s:%s:%u\n ", new_peer_name, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
          server_add_peer_to_database(new_peer_name, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
	  
	  char *captured;
          captured = strtok(buff, ",");
          for(;captured;captured = strtok(NULL, ","))
	  {
 	     printf("new file %s\n", captured);
          } 
	   
          sent_recv_bytes = recv(comm_socket_fd, (char *)buff, sizeof(buff), 0);
	  
	  int nnodes;
	  memcpy(&nnodes, buff, 4);
	  nnodes = ntohs(nnodes);

	  int i;
	  for (i = 0; i < nnodes; i++)
	  {	
            char peer_name[256] = {0}, peer_ip[16] = {0};
	    int peer_port;

            sent_recv_bytes = recv(comm_socket_fd, (char *)buff, sizeof(buff), 0);

	    sscanf(buff, "%[^:]:%[^:]:%d", peer_name, peer_ip, peer_port);
	    server_add_peer_to_database(peer_name, peer_ip, peer_port);
	  }

        }
        else if (option == 0)
        {
           server_send_file(comm_socket_fd);
        }
        else 
        {
   	  close(comm_socket_fd);
        }
	close(comm_socket_fd);
      } 
    }
  }
}

void setup_tcp_client_communication()
{
  int sockfd = 0, 
  sent_recv_bytes = 0;
  int addr_len = 0;
  addr_len = sizeof(struct sockaddr);

  struct sockaddr_in dest;
  dest.sin_family = AF_INET;
  dest.sin_port = SERVER_PORT;
  
  struct hostent *host = (struct hostent *)gethostbyname(SERVER_IP_ADDRESS);
  dest.sin_addr = *((struct in_addr *)host->h_addr);
  sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	// CONNECT TO THE SERVER
  
  connect(sockfd, (struct sockaddr *)&dest,sizeof(struct sockaddr));
  
  char option = client_choose_option();  
  client_send_option(sockfd, option);
  
  if (option == 1)
  {
    printf("syn request\n");
    char buff[1024] = {0};
    int sent_recv_bytes;
    sprintf(buff, "%s:%s:%d:", NODE_NAME, NODE_IP, NODE_PORT);
    //pritnf(buff);
  
    DIR *newd;
    struct dirent *dir;
    newd = opendir(".");
    if (newd)
    {
      while ((dir = readdir(newd)) != NULL)
      {
        strncat(buff, dir->d_name, 1024);
        strncat(buff, ",", 1024);
      }
      closedir(newd);
    }

    sent_recv_bytes = send(sockfd, &buff, sizeof(buff), 0);

    int nnodes = 0;
    /**
     * count nodes
     */
    nnodes = htons(nnodes);
    sent_recv_bytes = send(sockfd, &nnodes, sizeof(buff), 0);
    nnodes = 0;

    int i;
    for (i = 0; i < nnodes; i++)
    {
      /**
       * send nodes
       */
      sent_recv_bytes = send(sockfd, &buff, sizeof(buff), 0);
    }
  } 
  else if (option == 0) 
  {
    printf("file request\n");
    // CHANGE FILE NAME HERE
    client_request_file(sockfd, FILENAME);
  } 
  else 
    printf("Error with choosing an option\n");
}

int main(int argc, char **argv)
{
  char option = 'd';
  while(1)
  {
    printf("Run as a client or as a server?(c|s)\n");
    scanf(" %c", &option);
    if(option == 's')
    {
      setup_tcp_server_communication();
    } 
    else if (option = 'c')
    {
      setup_tcp_client_communication();
    } 
    else 
    {
      printf("Wrong option. (type only one character)\n");
      return -1;
    }
  }
  
  return 0;
}
