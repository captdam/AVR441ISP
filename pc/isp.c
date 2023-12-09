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

/** TCP write with error check. 
 * Write data to client; in case of any error, print error message. 
 * @param fd TCP file descriptor
 * @param data Data (0 terminated string) to write
*/
void tcp_writecheck(int fd, char* data) {
	if (write(fd, data, strlen(data)) < 0)
		perror("Fail to reply to TCP client");
}

/** Hot load HTML file to create a HTTP response (for dev use). 
 * This function will assume malloc always success. 
 * @param file HTML file name
 * 
*/
void tcp_sendfile(const char* html) {
	FILE* fd = fopen(html, "R");
	fseek(fd, 0, SEEK_END);
	long fsize = ftell(fd);
	rewind(fd);

	char* d = malloc(ftell(fd));
}

/** Send a byte through tty. 
 * If success, return NULL and write the rx value into rx; if failed, return a pointer to the error message. 
 * @param tty TTY file descriptor
 * @param tx Data to send
 * @param rx Data returned from tty (if success, this is value from tty device; if failed with mismatch echo, this is the bad echo)
 * @return NULL if success, otherwise pointer to error message (error message if always 13 character + 1 0-terminator, 14 bytes in tital)
*/
const char* tty_send(int tty, uint8_t tx, uint8_t* rx) {
	int whatever = write(tty, &tx, 1);
	tcdrain(tty);

	if (read(tty, rx, 1) != 1)
		return "Echo timedout";

	if (*rx != tx)
		return "Echo mismatch";

	if (read(tty, rx, 1) != 1)
		return "Data timedout";

	return NULL;
}


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
			uint32_t tx, rx = 0;
			if (!sscanf(buffer, "%*[^/]/%"SCNu32" ", &tx)) {
				puts("Bad POTS/PUT request from TCP client: Cannot get ISP data");
				tcp_writecheck(transaction, "HTTP/1.1 400 Bad Request\r\nContent-Length: 19\r\n\r\nCannot get ISP data");
				continue;
			}

			fprintf(stdout, ">0x%08X\n", tx);
			while (size == sizeof(buffer)) { size = read(transaction, buffer, sizeof(buffer)); } //Get rid of remaining HTTP data
			tcflush(tty, TCIOFLUSH);
			uint8_t t, r;
			const char* info;
			const char* message = "ISP error: %s >0x%02X, <0x%02X\n";

			t = (tx & 0xFF000000) >> 24;
			if (info = tty_send(tty, t, &r)) {
				fprintf(stdout, message, info, t, r);
				continue;
			}
			rx |= r << 24;

			t = (tx & 0x00FF0000) >> 16;
			if (info = tty_send(tty, t, &r)) {
				fprintf(stdout, message, info, t, r);
				continue;
			}
			rx |= r << 16;

			t = (tx & 0x0000FF00) >> 8;
			if (info = tty_send(tty, t, &r)) {
				fprintf(stdout, message, info, t, r);
				continue;
			}
			rx |= r << 8;

			t = tx & 0x000000FF;
			if (info = tty_send(tty, t, &r)) {
				fprintf(stdout, message, info, t, r);
				continue;
			}
			rx |= r;

			fprintf(stdout, "<0x%08X\n", rx);
			snprintf(buffer, sizeof(buffer), "HTTP/1.1 200 OK\r\nContent-Length: 10\r\n\r\n0x%08X", rx);
			tcp_writecheck(transaction, buffer);
			continue;
		}
			
		// GET - Index page
		if (buffer[0] == 'G') {
			puts("Request index page");
			while (size == sizeof(buffer)) { size = read(transaction, buffer, sizeof(buffer)); }

			#include "index.h"
			tcp_writecheck(transaction, __index_http);
			continue;
		}
		
		// Others - Close program
		puts("Request to halt program");
		while (size == sizeof(buffer)) { size = read(transaction, buffer, sizeof(buffer)); }

		tcp_writecheck(transaction, "HTTP/1.1 410 Gone\r\nContent-Length: 12\r\n\r\nService halt");
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