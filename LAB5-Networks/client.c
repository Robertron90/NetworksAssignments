#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <memory.h>
#include "common.h"

#define PORT            2000
#define SERVER "192.168.8.2"

student client_data;
result_struct_t result;

void die(char *s){
	perror(s);
}

#define BUFLEN 1024
void setup_tcp_communication() {
    struct sockaddr_in si_other;
	int s, i, slen=sizeof(si_other);
	char buf[BUFLEN];
	char message[BUFLEN];

	if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		die("socket");
	}

	memset((char *) &si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(PORT);

	if (inet_aton(SERVER , &si_other.sin_addr) == 0)
	{
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
	}
	student st;
	while(1)
	{
		printf("Enter student's name: ");
		scanf("%s",st.name);
		printf("Enter student's age: ");
		scanf("%u",&st.age);
		printf("Enter group: ");
		scanf("%s",st.group);

		//send the message
		if (sendto(s, &st, sizeof(st) , 0 , (struct sockaddr *) &si_other, slen)==-1)
		{
			die("sendto()");
		}
	}

	close(s);
}


int main(int argc, char **argv) {
    setup_tcp_communication();
    printf("done! \n");
    return 0;
}

