#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>
#include <errno.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <string.h>

#define MAX_WORD_LENGTH 20

/*Server process is running on this port no. Client has to send data to this port no*/
#define SERVER_PORT  2003
#define CLIENT_PORT  2077

#define FILENAME_REQUEST "test2.txt"
#define FILENAME   "----test.txt"

#define DB_NAME "db2.txt"

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int word_count(char* filename)
{
    FILE * f = fopen(filename, "r");
	if (f == NULL) return 1;
	char c;
	int count = 0;
	while((c = fgetc(f)) != EOF)
	{
		if(c == ' ' || c == '\n')
		{
			count++;
		}
	}
    fclose(f);
    return count;
}

char** convert_text_to_array(char* filename){
    int size = word_count(filename);
    FILE * fp = fopen(filename, "r");
    char **data = (char **)malloc(size * sizeof(char *));
		int i;
    for (i = 0; i < size; i++)
         data[i] = (char *)malloc(MAX_WORD_LENGTH * sizeof(int));
	if (fp == NULL) return 1;
	char c;
	int count = 0;
  i = -1;
	while((c = fgetc(fp)) != EOF)
	{
		if(c == ' ' || c == '\n')
		{
            data[count][++i] = '\0';
            count++;
            i = -1;
		}
		else
		{
            data[count][++i] = c;
		}
	}
	fclose(fp);
    return data;
}

char* my_ip(){

		struct ifaddrs *ifap, *ifa;
    struct sockaddr_in *sa;
    char *addr;
		char *ip = (char *)malloc(20 * sizeof(char));


    getifaddrs (&ifap);
    int i=0;
    for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr->sa_family==AF_INET) {
            sa = (struct sockaddr_in *) ifa->ifa_addr;
            addr = inet_ntoa(sa->sin_addr);
            if(i == 2)
                //printf("Interface: %s\tAddress: %s\n", ifa->ifa_name, addr);
								strcpy(ip, addr);
            i++;
        }

    }
    freeifaddrs(ifap);
    return ip;
}

void
tcp_server(char name[]){

   int master_sock_tcp_fd = 0,
       sent_recv_bytes = 0,
       addr_len = 0,
       opt = 1;

   
   int comm_socket_fd = 0;
   
   fd_set readfds;
   struct sockaddr_in server_addr, /*structure to store the server and client info*/
                      client_addr;

   if ((master_sock_tcp_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP )) == -1)
   {
       printf("socket creation failed\n");
       exit(1);
   }

   /*Step 3: specify server Information*/
   server_addr.sin_family = AF_INET;/*This socket will process only ipv4 network packets*/
   server_addr.sin_port = SERVER_PORT;/*Server will process any data arriving on port no 2000*/

   server_addr.sin_addr.s_addr = INADDR_ANY;

   addr_len = sizeof(struct sockaddr);

   if (bind(master_sock_tcp_fd, (struct sockaddr *)&server_addr, sizeof(struct sockaddr)) == -1)
   {
       printf("socket bind failed\n");
       return;
   }

   if (listen(master_sock_tcp_fd, 5)<0)
   {
       printf("listen failed\n");
       return;
   }

	 char data_buffer[1024];

   while(1){

       FD_ZERO(&readfds);                     /* Initialize the file descriptor set*/
       FD_SET(master_sock_tcp_fd, &readfds);  /*Add the socket to this set on which our server is running*/

       printf("blocked on select System call...\n");


      select(master_sock_tcp_fd + 1, &readfds, NULL, NULL, NULL);

       if (FD_ISSET(master_sock_tcp_fd, &readfds))
       {

           printf("New connection recieved recvd, accept the connection. Client and Server completes TCP-3 way handshake at this point\n");


           comm_socket_fd = accept(master_sock_tcp_fd, (struct sockaddr *)&client_addr, &addr_len);

           if(comm_socket_fd < 0){
               printf("accept error : errno = %d\n", errno);
               exit(0);
           }

           printf("Connection accepted from client : %s:%u\n",
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

           printf("Got new node! 'NAME':%s:%u\n",
                      inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

          //  while(1){
						 	 int sig = -1;
							
               memset(data_buffer, 0, sizeof(data_buffer));
							
							 //Recieve signal "0" or "1"
               sent_recv_bytes = recv(comm_socket_fd, (char *)data_buffer, sizeof(int), 0);

							 sig = atoi(data_buffer);

							 printf("Recived: %i | Bytes: %i\n", sig, sent_recv_bytes);
							
               if(sent_recv_bytes == 0){
               		close(comm_socket_fd);
               		break;
               }


							//If recieved 0, then Client will send inoformation about him and his peers in db
							if (sig == 1){

								 FILE* dbp = fopen(DB_NAME, "a"); //create new file and open	
								 memset(data_buffer, 0, sizeof(data_buffer));
								 //His info
								 char his_info[50];
							   sent_recv_bytes = recv(comm_socket_fd, (char *)data_buffer, sizeof(data_buffer), 0);
								 strcpy(his_info, data_buffer);
								 strcat(his_info, "\n");
								 printf("Recived: %s | Bytes: %i\n", data_buffer, sent_recv_bytes);
								 fputs(his_info, dbp);	

								 char int_buff[20];
								 //Number of peers in db
							   sent_recv_bytes = recv(comm_socket_fd, (char *)int_buff, sizeof(int), 0);
								 int num_nodes = atoi(int_buff);
								 printf("Recived: #nodes=%i | Bytes: %i\n", num_nodes, sent_recv_bytes);

								 //one-by-one peer sending
								 printf("--------------\n");
								 int i;
								 char peer_buff[50];

								 int bytes;
								 for(i=0; i< num_nodes; i++){
										  bytes = recv(comm_socket_fd, (char *)peer_buff, sizeof(char)*50, 0);
											sleep(1);
											fputs(peer_buff,dbp);
											printf("%s\n", peer_buff);
								 }
								 		fclose(dbp);
								 	 printf("------------\n");
							 	
							}else if (sig == 0){
								//server_send_file(comm_socket_fd);
								 char text_file[50];
								 printf("Request for sending file RECIEVED\n");	

								 sent_recv_bytes = recv(comm_socket_fd, (char *)data_buffer, sizeof(data_buffer), 0);
								 strcpy(text_file, data_buffer);
								 printf("Recived: %s | Bytes: %i\n", text_file, sent_recv_bytes);
								 //strcat(text_file, ".txt");
							  //  strcpy(text_file, data_buffer);

								 FILE* fp = fopen(text_file, "r");

								 int msg =-1;
								 int str_msg;
								 char **data;

								 if(fp != NULL){
									 	msg = word_count(text_file);
										data = convert_text_to_array(text_file); 
								 }else{
									 printf("hasn't this file: %s\n", text_file);
								 }
								 
								 printf("-------------\n");
								 memcpy(&str_msg, &msg, 4);
								 sent_recv_bytes = send(comm_socket_fd, &str_msg, sizeof(int), 0);	

								 int i;
								 char word[25]; 
								 for(i =0; (fp!=NULL && i< msg); i++){
									 strcpy(word, data[i]);
									 sent_recv_bytes = send(comm_socket_fd, &word, sizeof(word), 0);
									 printf("Sended: %s | Bytes: %d\n", word, sent_recv_bytes);	
									 sleep(1);
								 }
								 printf("-------------\n");	
								 fclose(fp);
           			}
					//  }
       }
   }
}


///////////// CLIENT /////////////////////
void tcp_client(char name[], char server_ip[], int server_port, int sig) {
		
    int sockfd = 0,
        sent_recv_bytes = 0;

    int addr_len = 0;
    addr_len = sizeof(struct sockaddr);

    struct sockaddr_in dest,
										my_addr;
    dest.sin_family = AF_INET;

    dest.sin_port = SERVER_PORT;
		struct hostent *host = (struct hostent *)gethostbyname(server_ip);
    dest.sin_addr = *((struct in_addr *)host->h_addr);
		
    sockfd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

		my_addr.sin_family = AF_INET; 
    my_addr.sin_addr.s_addr = INADDR_ANY; 
    my_addr.sin_port = htons(CLIENT_PORT); 
      
    if (bind(sockfd, (struct sockaddr*) &my_addr, sizeof(struct sockaddr_in)) == 0) 
        printf("Binded Correctly\n"); 
    else
        printf("Unable to bind\n"); 
      
    socklen_t addr_size = sizeof my_addr; 

    connect(sockfd, (struct sockaddr *)&dest, sizeof(struct sockaddr));

		char* myIP = my_ip();
		char data_buffer[1024];

			char request_file[9] = "test.txt";
			char answer[6];

			int sync = 1;
			
			int req_file = 0;
			int num_of_nodes=0;
			int int_buf;

			int next_word = 0;

			if(sig == sync){
						//Send Sync signal
					memcpy(&int_buf, &sync, 4);
					sent_recv_bytes = send(sockfd, (char*)&int_buf, sizeof(int), 0);
					printf("Sended: %d | No of bytes sent = %d\n", sync, sent_recv_bytes);
					
					char my_info[100];
					char my_port[6];
					snprintf(my_port,6, "%d", CLIENT_PORT);
					strcpy(my_info, name);
					strcat(my_info, ":");
					strcat(my_info, myIP); //send my IP
					strcat(my_info, ":");
					strcat(my_info, my_port); //send my PORT
					strcat(my_info, ":");
					strcat(my_info, "[");
					//filenames of files that you have
					strcat(my_info, "]");

					//Send my informtion about me to another peer
					sent_recv_bytes = send(sockfd, &my_info,
					sizeof(my_info),  //May be not that
					0);

					printf("Sended: %s | No of bytes sent = %d\n", my_info, sent_recv_bytes);
					
					//Send number of nodes in your db to another peer
					num_of_nodes = word_count("db.txt");
					memcpy(&int_buf, &num_of_nodes, 4);
					sent_recv_bytes = send(sockfd, (char*)&int_buf, sizeof(int), 0);
					printf("Sended: number of nodes = %d | No of bytes sent = %d\n", num_of_nodes, sent_recv_bytes);

					//one by one send information about every node to the peer
					char **data = convert_text_to_array("db.txt");
					int i;
					char peer[50];
					printf("------------------\n");
					for (i= 0; i < num_of_nodes; i++){
							sleep(1);
							printf("%s\n", data[i]);
							strcpy(peer, data[i]);
							sent_recv_bytes = send(sockfd, &peer,
									sizeof(char)*50,  //May be not that
									0);
							printf("No of bytes sent = %d\n", sent_recv_bytes);
					}
					printf("------------------\n");

			}else if (sig == req_file){

					int num_of_words = 0;
				
					char send_msg[100];

  				strcpy(send_msg, FILENAME);
 					sent_recv_bytes = send(sockfd, &send_msg, sizeof(send_msg), 0);
					printf("Requested: %s | No of bytes sent = %d\n", FILENAME, sent_recv_bytes);
				
					sent_recv_bytes = recv(sockfd, (char *)data_buffer, sizeof(data_buffer), 0);\
					num_of_words = atoi(data_buffer);
					printf("Recived: %d | Bytes: %d\n", num_of_words, sent_recv_bytes);
					
					//Compare OK with recieved message
					if(num_of_words == -1 || num_of_words == 0){
						printf("Server hasn't this file :(\n");
						exit(-2);
					}else{

							FILE* fp = fopen(FILENAME_REQUEST, "w"); //create new file and open
							printf("-----------\n");
							if(fp == NULL) {printf("Unenable to create file\n"); exit(-3); }
							int i;
							char word[25];
							for(i =0; i< num_of_words; i++){
									memset(data_buffer, 0, sizeof(data_buffer));
									sent_recv_bytes = recv(sockfd, (char *)word, sizeof(word), 0);
									//strcpy(word, data_buffer);
									printf("Recived: %s | Bytes: %d\n", word, sent_recv_bytes);
									fputs(word, fp);
									if( i < num_of_words - 1) fputs(" ", fp);
							}
							fclose(fp); //close the file
							printf("----------\n");

							printf("TRANSFER COMLETE. \n");
			}

    }
}

int
main(int argc, char **argv){

	 char name[100] = "Default";
	 char peer;
	 printf("Name of peer: ");
	 while(1){
		 	scanf(" %[^\n]", name);
			printf("Run Peer as Server or as Client??\nwrite 'c' for client, 's' for the server\n");

			scanf("%c", &peer);

   

			if(peer == 'c'){
					printf("Please enter NAME: \n");
				
					printf("Please enter IP of your potential connection partner\n");
					char IP[16];
					scanf(" %[^\n]", IP);
					printf("Please enter PORT of your potential connection partner\n");
					int PORT;
					scanf(" %d", &PORT);
					int KEY;
					printf("Sync or request file?([1] or [0])\n");
					scanf(" %d", &KEY);
					tcp_client(name, IP, PORT, KEY);
			}
			else if (peer = 's'){
				tcp_server(name);
			}
   }
   return 0;
}
