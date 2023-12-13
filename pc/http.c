#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include "http.h"

int fd = 0;
char buffer[64]; //Should be small enough for PC's stack size, but large enough for our HTTP request

int http_init(int port) {
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("Cannot open TCP");
		return errno;
	}

	struct sockaddr_in tcpServer = {
		.sin_family = AF_INET,
		.sin_port = htons(port),
		.sin_addr.s_addr = INADDR_ANY
	};
	if (bind(fd, (struct sockaddr*)&tcpServer, sizeof(tcpServer)) < 0) {
		perror("Cannot bind TCP socket");
		http_close();
		return errno;
	}

	if (listen(fd, 1) < 0) {
		perror("Fail to start to listen on TCP");
		http_close();
		return errno;
	}

	return 0;
}

int http_receive(char** method, char** data) {
	int transaction = accept(fd, NULL, NULL);
	if (transaction < 0) {
		perror("Bad request (accept failed)");
		return -1;
	}

	ssize_t size = read(transaction, buffer, sizeof(buffer));
	if (size <= 0) {
		perror("Bad request (bad size)");
		return -1;
	}

	buffer[sizeof(buffer)-1] = '\0';
	char* space = strchr(buffer, ' ');
	if (space && space < buffer + (sizeof(buffer)>>1)) {
		*method = buffer;
		*space = '\0';
		*data = space + 1; //METHOD /URL
	}
	return transaction;
}

void http_send(int transaction, char* code, void* data, int len) {
	char header[64];
	sprintf(header, "HTTP/1.1 %s\r\nContent-Length: %d\r\n\r\n", code, len);
	send(transaction, header, strlen(header), MSG_MORE);
	if (write(transaction, data, len) < 0)
		perror("Fail to reply to TCP client");
	close(transaction);
}

void http_close() {
	if (fd > 0) {
		fputs("Close server\n", stderr);
		close(fd);
		fd = 0;
	}
}