#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

const int CONNECTION_COUNT = 1;

const char* SERVER_ADDRESS = "google.se"; /*"127.0.0.1";*/
const char* SERVER_PORT = "8000";

int main(int argc, char** argv) {

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
	for (int s = 0; s < CONNECTION_COUNT; ++s) {
		int sock_fd = socket(addrinfo->ai_family, addrinfo->ai_socktype, addrinfo->ai_protocol);
		if (0 > sock_fd) {
			perror("socket");
			return -4;
		}

		fprintf(stderr, "socket: %d\n", sock_fd);
		fprintf(stderr, "addr: %s\n", inet_ntoa(inet_makeaddr(addrinfo->ai_addr->sa_data, addrinfo->ai_addr->sa_len)));
		if(connect(sock_fd, addrinfo->ai_addr, addrinfo->ai_addrlen)) {
			perror("connect");
			return -5;
		}

		sock_fds[s] = sock_fd;
	}

	fprintf(stderr, "Closing sockets...\n");
	for (int s = 0; s < CONNECTION_COUNT; ++s) {
		close(sock_fds[s]);
	}

	return 0;
}
