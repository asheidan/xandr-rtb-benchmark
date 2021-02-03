#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include <argp.h>

const char* MESSAGE = "GET /category?url=ojoj.se HTTP1.1\n\n";

const char *argp_program_version = "Xandr listener benchmark v. 0.0.0";
const char *argp_program_bug_address = "support@adlede.com";
static const char * const doc = "Program description.";
/*static const char * const args_doc = "[FOO=?]...";*/

static struct argp_option options [] = {
	{ 0, 0, 0, 0, "Benchmark options", 1 },
	{ "connections", 'c', "COUNT", 0, "Number of simultaneous connections to open to the listener.", 1 },
	{ "messages", 'm', "COUNT", 0, "Number of messages to send to the listener", 1 },

	{ 0, 0, 0, 0, "Listener options", 2 },
	{ "port", 'p', "PORT", 0, "Portnumber for the listener. (default 8080)", 2 },
	{ "host", 'h', "ADDRESS", 0, "Address to the listener. (default 127.0.0.1)", 2 },

	{ 0, 0, 0, 0, "Other options", -1 },
	{ 0 },
};
struct arguments {
	int connection_count;
	int message_count;

	char* server_port;
	char* server_address;
};

/**
 * Parse callback for the argp parser.
 *
 * Arguments are documented in the argp documentation
 */
static error_t parse_opt(int key, char* arg, struct argp_state *state) {
	struct arguments *arguments = state->input;

	switch (key) {
		case 'c':
			arguments->connection_count = atoi(arg);
			if (arguments->connection_count < 1) {
				argp_error(state, "Connection count needs to be a positive integer\n");
			}
			break;
		case 'm':
			arguments->message_count = atoi(arg);
			if (arguments->message_count < 1) {
				argp_error(state, "Message count needs to be a positive integer\n");
			}
			break;

		case 'p':
			arguments->server_port = arg;
			break;
		case 'h':
			arguments->server_address = arg;
			break;

		case ARGP_KEY_ARG:
			return 0;
		default:
			return ARGP_ERR_UNKNOWN;
	}

	return 0;
}

static struct argp argp = { options, parse_opt, 0 /*args_doc*/, doc, 0, 0, 0 };


/* Subtract the ‘struct timeval’ values X and Y,
 * storing the result in RESULT.
 *
 * RETURN 1 if the difference is negative, otherwise 0.
 *
 * https://www.gnu.org/software/libc/manual/html_node/Calculating-Elapsed-Time.html
 */
int timeval_subtract (const struct timeval *x, struct timeval *y, struct timeval *result)
{
	/* Perform the carry for the later subtraction by updating y. */
	if (x->tv_usec < y->tv_usec) {
		int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
		y->tv_usec -= 1000000 * nsec;
		y->tv_sec += nsec;
	}
	if (x->tv_usec - y->tv_usec > 1000000) {
		int nsec = (x->tv_usec - y->tv_usec) / 1000000;
		y->tv_usec += 1000000 * nsec;
		y->tv_sec -= nsec;
	}

	/* Compute the time remaining to wait.
	   tv_usec is certainly positive. */
	result->tv_sec = x->tv_sec - y->tv_sec;
	result->tv_usec = x->tv_usec - y->tv_usec;

	/* Return 1 if result is negative. */
	return x->tv_sec < y->tv_sec;
}

int main(int argc, char** argv) {
	struct arguments arguments;
	arguments.connection_count = 128;
	arguments.message_count = 100000;
	arguments.server_address = "127.0.0.1";
	arguments.server_port = "8080";

	argp_parse(&argp, argc, argv, 0, 0, &arguments);

	const size_t buf_size = 512;  /* TODO: Expose this as a setting */
	char recv_buf[buf_size];
	int sock_fds[arguments.connection_count];

	struct timeval starttime, endtime, runtime;

	struct addrinfo hints, *addrinfo;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	int gai_error = getaddrinfo(arguments.server_address, arguments.server_port, &hints, &addrinfo);
	if (gai_error) {
		fprintf(stderr, "Fail: %s\n", gai_strerror(gai_error));
		return -42;
	}

	fprintf(stderr, "Address: %s:%s\n", inet_ntoa(((struct sockaddr_in*)addrinfo->ai_addr)->sin_addr), arguments.server_port);
	fprintf(stderr, "Opening %d sockets...\n", arguments.connection_count);
	for (int s = 0; s < arguments.connection_count; ++s) {
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
	fprintf(stderr, "Sending %d messages...\n", arguments.message_count, arguments.message_count);
	gettimeofday(&starttime, NULL);
	for (int i = 0; i < arguments.message_count; ++i) {
		int sock_num = rand() % arguments.connection_count;
		int sock_fd = sock_fds[sock_num];
		int send_count = send(sock_fd, MESSAGE, message_len, 0);

		poll_struct.fd = sock_fd;
		int poll_ret = poll(&poll_struct, 1, -1);
		int recv_count = read(sock_fd, recv_buf, buf_size);
	}
	gettimeofday(&endtime, NULL);
	timeval_subtract(&endtime, &starttime, &runtime);

	fprintf(stderr, "Closing sockets...\n");
	for (int s = 0; s < arguments.connection_count; ++s) {
		close(sock_fds[s]);
	}

	fprintf(stdout, "%4d %7d %ld.%06ld\n",
			arguments.connection_count, arguments.message_count,
			runtime.tv_sec, runtime.tv_usec);

	return 0;
}
