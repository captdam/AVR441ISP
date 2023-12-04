#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <arpa/inet.h> 
#include <netinet/in.h> 

#define ISP_BAUD B9600

void write_echeck(int fd, char* data) { if (write(fd, data, strlen(data)) < 0) perror("Fail to reply to TCP client"); }


int main(int argc, char* argv[]) {
	if (argc != 3) {
		fprintf(stderr, "Invalid argument. Use %s TTY TCP\n", argv[0]);
		return -1;
	}

	// Open TTY interface
	fprintf(stderr, "Open UART TTY: %s\n", argv[1]);
	int tty = open(argv[1], O_RDWR | O_NOCTTY);
	if (tty < 0) {
		perror("Cannot open TTY");
		goto exit;
	}
	if (!isatty(tty)) {
		fprintf(stderr, "%s is not a tty device\n", argv[1]);
		goto exit;
	}
	
	// Open TCP server
	fprintf(stderr, "Open server on tcp://localhost:%s (http)\n", argv[2]);
	int tcp = socket(AF_INET, SOCK_STREAM, 0);
	if (tcp < 0) {
		perror("Cannot open TCP");
		goto exit;
	}
	struct sockaddr_in tcpServer = {
		.sin_family = AF_INET,
		.sin_port = htons(atoi(argv[2])),
		.sin_addr.s_addr = INADDR_ANY
	};
	if (bind(tcp, (struct sockaddr*)&tcpServer, sizeof(tcpServer)) < 0) {
		perror("Cannot bind TCP socket");
		goto exit;
	}
	fputs("Start to listen on TCP\n", stderr);
	if (listen(tcp, 1) < 0) {
		perror("Fail to start to listen on TCP");
		goto exit;
	}

	// Configure TTY
	struct termios ttyBackup, ttyCurrent = {
		.c_cflag = ISP_BAUD | CS8 | /*PARENB | */CLOCAL | CREAD, //8-bit data with parity (to support UPDI, our ISP ignore frame error), disable modem control
		.c_iflag = IGNBRK | IGNPAR //Ignor line brake and parity error
	};
	ttyCurrent.c_cc[VTIME] = 1; //Read timeout 0.1s
	ttyCurrent.c_cc[VMIN] = 0; //Read success when any length of data received from ISP
	tcflush(tty, TCIOFLUSH);
	tcgetattr(tty, &ttyBackup);
	tcsetattr(tty, TCSANOW, &ttyCurrent);

	// Main
	for(int transaction = 0;;) {
		char buffer[256]; // Should be small enough for PC's stack size, but large enough for our HTTP request
		uint8_t tx, rx;
		ssize_t size;

		if (transaction >= 0)
			close (transaction); //Close connection. No keep-alive, local socket is fast, keep-alive connection only makes our program complex
		
		if (transaction = accept(tcp, NULL, NULL) < 0) {
			perror("Bad connection from TCP client");
			continue;
		}
		
		if (size = read(transaction, buffer, sizeof(buffer)) <= 0) {
			perror("Bad request from TCP client");
			continue;
		}

		// POST / PUT - ISP data
		if (buffer[0] == 'P') {
			if (!sscanf(buffer, "%*[^/]/%"SCNu8" ", &tx)) {
				puts("Bad POTS/PUT request from TCP client: Cannot get ISP data");
				write_echeck(transaction, "HTTP/1.1 400 Bad Request\r\nContent-Length: 19\r\n\r\nCannot get ISP data");
				continue;
			}

			fprintf(stdout, ">0x%X (%"PRIu8")\n", tx, tx);
			while (size == sizeof(buffer)) { size = read(transaction, buffer, sizeof(buffer)); } //Get rid of remaining HTTP data
			tcflush(tty, TCIOFLUSH);

			// Send data to ISP
			write(tty, &tx, 1);
			tcdrain(tty);

			// Receive data from ISP - echo
			if (read(tty, &rx, 1) != 1) {
				puts(">ISP ERROR");
				write_echeck(transaction, "HTTP/1.1 504 Gateway Timeout\r\nContent-Length: 18\r\n\r\nISP offline - echo");
				continue;
			}

			if (rx != tx) {
				fprintf(stdout, "ISP echo mismatched, send %2X, echo %2X", tx, rx);
				write_echeck(transaction, "HTTP/1.1 502 Bad Gateway\r\nContent-Length: 31\r\n\r\nEcho mismatched: send %2X get %2X");
				continue;
			}

			// Receive data from ISP - AVR
			if (read(tty, &rx, 1) != 1) {
				puts(">ISP ERROR");
				write_echeck(transaction, "HTTP/1.1 504 Gateway Timeout\r\nContent-Length: 17\r\n\r\nISP offline - avr");
				continue;
			}

			fprintf(stdout, "<0x%X (%"PRIu8")\n", rx, rx);
			snprintf(buffer, sizeof(buffer), "HTTP/1.1 200 OK\r\nContent-Length: 3\r\n\r\n%3"PRIu8, rx);
			write_echeck(transaction, buffer);
			continue;
		}
			
		// GET - Index page
		if (buffer[0] == 'G') {
			puts("Request index page");
			while (size == sizeof(buffer)) { size = read(transaction, buffer, sizeof(buffer)); }

			#include "index.h"
			write_echeck(transaction, __index_http);
			continue;
		}
		
		// Others - Close program
		puts("Request to halt program");
		while (size == sizeof(buffer)) { size = read(transaction, buffer, sizeof(buffer)); }

		write_echeck(transaction, "HTTP/1.1 410 Gone\r\nContent-Length: 12\r\n\r\nService halt");
		close(transaction);
		break;
	}
	tcsetattr(tty, TCSANOW, &ttyBackup);

	exit:
	if (tcp > 0) {
		fputs("Close server\n", stderr);
		close(tcp);
		tcp = 0;
	}
	if (tty > 0) {
		fputs("Restore and close TTY\n", stderr);
		close(tty);
	}

	return 0;
}