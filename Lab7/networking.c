#include "networking.h"

void get_ip_port(int sockfd, int *port, char **ipaddr) {
	struct sockaddr_storage address;
	socklen_t len;
	getpeername(sockfd, (struct sockaddr *) &address, &len);

	struct sockaddr_in *addr = (struct sockaddr_in *) &address;
	*port = ntohs(addr->sin_port);
	inet_ntop(AF_INET, &addr->sin_addr, (char *) ipaddr, sizeof(ipaddr) * INET_ADDRSTRLEN);
}

int init_tcp_server() {
	int socket_listen = 0;
	socket_listen = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (socket_listen < 0) {
		perror("socket creation failed.\n");
		return -1;
	}

	struct sockaddr_in addr_resp;

	addr_resp.sin_addr.s_addr = INADDR_ANY;
	addr_resp.sin_port = SERVER_PORT;
	addr_resp.sin_family = AF_INET;

	int result = 0;

	result = bind(socket_listen, (struct sockaddr *) &addr_resp, sizeof(struct sockaddr));
	if (result < 0) {
		perror("Socket was unable to bind.\n");
		return -1;
	}

	return socket_listen;
}

int setup_communication(int conn_socket) {
	int result = 0;
	result = listen(conn_socket, 8);
	if (result < 0) {
		return -1;
	}

	struct sockaddr_in addr_resp;

	addr_resp.sin_family = AF_INET;
	addr_resp.sin_port = SERVER_PORT;
	addr_resp.sin_addr.s_addr = INADDR_ANY;

	int socket_talk = 0;
	socklen_t addr_len = sizeof(struct sockaddr);
	socket_talk = accept(conn_socket, (struct sockaddr *) &addr_resp, &addr_len);
	if (socket_talk < 0) {
		return -1;
	}

	return socket_talk;
}

int recvsend(int conn_socket, void *readbuf, size_t readlen, void *sendbuf, size_t sendlen) {
	if (conn_socket < 0) {
		perror("Invalid socket value.\n");
		return -1;
	}

	long int result = 0;

	result = read(conn_socket, readbuf, readlen);
	if (result < 0) {
		perror("Read error.\n");
		return -1;
	}

	result = send(conn_socket, sendbuf, sendlen, 0);
	if (result < 0) {
		perror("Send error.\n");
		return -1;
	}

	return 0;
}

int sendrecv(int conn_socket, void *sendbuf, size_t sendlen, void *readbuf, size_t readlen) {
	if (conn_socket < 0) {
		perror("Invalid socket value.\n");
		return -1;
	}

	long int result = 0;

	result = send(conn_socket, sendbuf, sendlen, 0);
	if (result < 0) {
		perror("Send error.\n");
		return -1;
	}

	result = read(conn_socket, readbuf, readlen);
	if (result < 0) {
		perror("Read error.\n");
		return -1;
	}

	return 0;
}

int ping(int conn_socket, int n) {
	int success = 0;
	int result = 0;

	int sendbuf = 0, readbuf = -1;
	for (size_t i = 0; i < n; i++) {
		if (conn_socket < 0) {
			printf("Cannot ping.\n");
			continue;
		}

		result = sendrecv(conn_socket, &sendbuf, sizeof(int), &readbuf, sizeof(int));
		if (result < 0) {
			printf("Cannot send/recieve message.\n");
			readbuf = -1;
			continue;
		}

		if (readbuf == 0) {
			success++;
			printf("Sent and recieved %lu bytes.\n", sizeof(int));
			readbuf = -1;
		}

		readbuf = -1;
	}

	return success;
}
