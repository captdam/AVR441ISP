#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <termios.h>
#include "http.h"

#define ISP_BAUD B115200

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
	
	fprintf(stderr, "Open server on tcp://localhost:%s (http)\n", argv[2]);
	http_init(atoi(argv[2]));

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
	for(;;) {
		char* transaction_method;
		char* transaction_data;
		int transaction = http_receive(&transaction_method, &transaction_data);
		if (transaction < 0) {
			continue;
		} else if (!transaction_method || !transaction_data) {
			puts("Bad request from TCP client: Cannot get method / data");
			http_send(transaction, "400 Bad Request", "Cannot get method / data", 25);
			continue;
		} else if (transaction_method[0] == 'D') {
			puts("Request to halt program");
			http_send(transaction, "410 Gone", "Server halt", 11);
			close(transaction);
			break;
		}

		// POST / PUT - ISP data
		if (transaction_method[0] == 'P') {
			uint32_t tx, rx = 0;
			if (!sscanf(transaction_data, "/%"SCNu32" ", &tx)) {
				puts("Bad POTS/PUT request from TCP client: Cannot get ISP data");
				http_send(transaction, "400 Bad Request", "Cannot get ISP data", 19);
				continue;
			}

			fprintf(stdout, ">0x%08X\n", tx);
			tcflush(tty, TCIOFLUSH);
			uint8_t t, r;
			const char* info;
			char buffer[64];

			t = (tx & 0xFF000000) >> 24;
			if (info = tty_send(tty, t, &r)) {
				snprintf(buffer, sizeof(buffer), "ISP error (byte 0): %s >0x%02X, <0x%02X", info, t, r);
				puts(buffer);
				http_send(transaction, "502 Gateway Error", buffer, strlen(buffer));
				continue;
			}
			rx |= r << 24;

			t = (tx & 0x00FF0000) >> 16;
			if (info = tty_send(tty, t, &r)) {
				snprintf(buffer, sizeof(buffer), "ISP error (byte 1): %s >0x%02X, <0x%02X", info, t, r);
				puts(buffer);
				http_send(transaction, "502 Gateway Error", buffer, strlen(buffer));
				continue;
			}
			rx |= r << 16;

			t = (tx & 0x0000FF00) >> 8;
			if (info = tty_send(tty, t, &r)) {
				snprintf(buffer, sizeof(buffer), "ISP error (byte 2): %s >0x%02X, <0x%02X", info, t, r);
				puts(buffer);
				http_send(transaction, "502 Gateway Error", buffer, strlen(buffer));
				continue;
			}
			rx |= r << 8;

			t = tx & 0x000000FF;
			if (info = tty_send(tty, t, &r)) {
				snprintf(buffer, sizeof(buffer), "ISP error (byte 3): %s >0x%02X, <0x%02X", info, t, r);
				puts(buffer);
				http_send(transaction, "502 Gateway Error", buffer, strlen(buffer));
				continue;
			}
			rx |= r;

			fprintf(stdout, "<0x%08X\n", rx);
			snprintf(buffer, sizeof(buffer), "0x%08X", rx);
			http_send(transaction, "200 OK", buffer, 10);
			
		// GET - Index page
		} else if (transaction_method[0] == 'G') {
			char filename[64];
			if (!sscanf(transaction_data, "/%s", filename)) {
				puts("Bad GET request from TCP client: Cannot get filen ame");
				http_send(transaction, "400 Bad Request", "Cannot get file name", 20);
				continue;
			}

			char filenameFullpath[16 + sizeof(filename)] = "./parts/";
			strncat(filenameFullpath, filename, sizeof(filename));
			fprintf(stdout, "Request page: %s\n", filenameFullpath);
			int file = open(filenameFullpath, O_RDONLY);
			if (file < 0) {
				perror("Cannot open webpage file");
				http_send(transaction, "404 Not Found", "", 0);
				continue;
			}
			struct stat fileinfo;
			if (fstat(file, &fileinfo)) {
				perror("Cannot get webpage info");
				http_send(transaction, "404 Not Found", "", 0);
				continue;
			}
			void* filedata = mmap(NULL, fileinfo.st_size, PROT_READ, MAP_PRIVATE, file, 0);
			if (filedata == MAP_FAILED) {
				perror("Cannot map webpage data");
				http_send(transaction, "404 Not Found", "", 0);
				continue;
			}

			fprintf(stdout, "Page OK, size: %d\n", fileinfo.st_size);
			http_send(transaction, "200 OK", filedata, fileinfo.st_size);

			if (munmap(filedata, fileinfo.st_size)) {
				perror("Cannot unmap webpage data (error ignored)");
			}
		
		// Others - Close program
		} else {
			puts("Cannot understand request");
			http_send(transaction, "501 Not Implemented", "Use GET, PUT / POST and DELETE only", 35);
		}
	}
	tcsetattr(tty, TCSANOW, &ttyBackup);

	exit:
	http_close();
	if (tty > 0) {
		fputs("Restore and close TTY\n", stderr);
		close(tty);
	}

	return 0;
}