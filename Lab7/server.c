#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include "networking.h"

void *node_handler(void *args) {
	int c_socket = *((int *) args);
	int alive = 1;

	while (alive) {
		alive = ping(c_socket, 1);
		sleep(1);
	}

	int port;
	char ipaddr[INET_ADDRSTRLEN];
	get_ip_port(c_socket, &port, (char **) ipaddr);
	printf("Node %s:%d  is left.\n", ipaddr, port);
}

void *connections_handler() {
	int master_socket = init_tcp_server();

	while (1) {
		int sockfd = setup_communication(master_socket);
		if (sockfd < 0) {
			perror("Cannot connect to TCP");
			continue;
		}

		int result = 0;

		int readbuf = 0;
		int sendbuf = 0;
		result = recvsend(sockfd, (void *) &readbuf, sizeof(int), &sendbuf, sizeof(int));
		close(sockfd);
		if (result < 0) {
			perror("Messages cannot be sent.\n");
			//exit(EXIT_FAILURE);
		}

		if (readbuf == 0) {
			send(sockfd, (const void *) &sendbuf, sizeof(int), 0);
		}

		if (readbuf == 1) {
			int port = 0;
			char ipaddress[INET_ADDRSTRLEN];
			get_ip_port(sockfd, &port, (char **) ipaddress);
			printf("New peer added with ip address %s and port %d", ipaddress, port);
			pthread_t new_thread;
			pthread_create(&new_thread, NULL, node_handler, (void *) &sockfd);
		}
	}
}

int main(int argc, char *argv[]) {

	if (argc > 1) {
		if (strcmp(argv[1], "-main") == 0) {
			pthread_t handler_thread = 0;
			pthread_create(&handler_thread, NULL, connections_handler, NULL);
		}
	}

	return 0;
}
