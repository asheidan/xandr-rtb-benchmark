#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

const int CONNECTION_COUNT = 128;
const int MESSAGE_COUNT = 100000;

const char* MESSAGE = "GET /category?url=ojoj.se HTTP1.1\n\n";

const char* SERVER_ADDRESS = "127.0.0.1";
const char* SERVER_PORT = "8080";


int main(int argc, char** argv) {

	const size_t buf_size = 512;
	char recv_buf[buf_size];
	int sock_fds[CONNECTION_COUNT];

	struct addrinfo hints, *addrinfo;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	int gai_error = getaddrinfo(SERVER_ADDRESS, SERVER_PORT, &hints, &addrinfo);
	if (gai_error) {
		fprintf(stderr, "Faile: %s\n", gai_strerror(gai_error));
		return -42;
	}

	fprintf(stderr, "Opening sockets...\n");
	fprintf(stderr, "addr: %s\n", inet_ntoa(((struct sockaddr_in*)addrinfo->ai_addr)->sin_addr));
	for (int s = 0; s < CONNECTION_COUNT; ++s) {
		int sock_fd = socket(addrinfo->ai_family, addrinfo->ai_socktype, addrinfo->ai_protocol);
		if (0 > sock_fd) {
			perror("socket");
			return -4;
		}

		if(connect(sock_fd, addrinfo->ai_addr, addrinfo->ai_addrlen)) {
			perror("connect");
			return -5;
		}

		sock_fds[s] = sock_fd;
	}

	size_t message_len = strlen(MESSAGE);
	struct pollfd poll_struct;
	poll_struct.events = POLLIN;
	poll_struct.revents = 0;
	fprintf(stderr, "Sending messages...\n");
	for (int i = 0; i < MESSAGE_COUNT; ++i) {
		int sock_num = rand() % CONNECTION_COUNT;
		int sock_fd = sock_fds[sock_num];
		int send_count = send(sock_fd, MESSAGE, message_len, 0);

		poll_struct.fd = sock_fd;
		int poll_ret = poll(&poll_struct, 1, -1);
		int recv_count = read(sock_fd, recv_buf, buf_size);
	}

	fprintf(stderr, "Closing sockets...\n");
	for (int s = 0; s < CONNECTION_COUNT; ++s) {
		close(sock_fds[s]);
	}

	return 0;
}
