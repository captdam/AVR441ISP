#define PIN_DI 0
#define PIN_DO 1
#define PIN_CK 2
#define PIN_UART 3
#define PIN_LED 4
#define UART_DELAY 104 // 1MHz / 9600BAUD = 104.16667 cycles

#include <avr/io.h>

uint8_t spi(uint8_t data);
void uart_tx(uint8_t data);
uint8_t uart_rx();

void main(void) {
	PORTB = (1<<PIN_UART) | (1<<PIN_DI); // Pull-up for idle state, pull-up for input level shifter
	DDRB = (1<<PIN_CK) | (1<<PIN_DO); // SPI output

	TCCR0A = (2<<CS00); // CTC mode with OCR0A
	OCR0A = UART_DELAY; // Output BAUD
	OCR0B = UART_DELAY >> 1; // Sample at middle

	for (uint8_t d;;) {
		d = uart_rx();
		d = spi(d);
		uart_tx(d);
	}
}

uint8_t spi(uint8_t data) {
	TCNT0 = 0; // Reset and start timer at clk/1 (1MHz)
	TIFR = (1<<OCF0A);
	TCCR0B = (1<<CS00);

	USIDR = data;
	USISR = (1<<USIOIF);
	do {
		while (!( TIFR & (1<<OCF0A) ));
		TIFR = (1<<OCF0A);
		USICR = (1<<USIWM0) | (1<<USICS1) | (1<<USICLK) | (1<<USITC);
	} while (!( USISR & (1<<USIOIF) ));
	return USIDR;
}

/* Low speed UART TX */
void uart_tx(uint8_t data) {
	TCNT0 = 0; // Reset and start timer, UART drive mode
	TIFR = (1<<OCF0A);
	TCCR0B = (1<<CS00);
	DDRB |= (1<<PIN_UART);

	PORTB &= ~(1<<PIN_UART); // 1 start bit
	while (!( TIFR & (1<<OCF0A) ));
	TIFR = (1<<OCF0A);
	
	for (uint8_t c = 8; c; c--) { // 8 data bits
		if (data & 1) {
			PORTB |= (1<<PIN_UART);
		} else {
			PORTB &= ~(1<<PIN_UART);
		}
		data = data >> 1;
		while (!( TIFR & (1<<OCF0A) ));
		TIFR = (1<<OCF0A);
	}

	PORTB |= (1<<PIN_UART); // 2 stop bits
	while (!( TIFR & (1<<OCF0A) ));
	TIFR = (1<<OCF0A);
	while (!( TIFR & (1<<OCF0A) ));
	
	DDRB &= ~(1<<PIN_UART); // Switch back to input mode & Stop timer to save energy
	TCCR0B = (0<<CS00);
}

/* Low speed UART RX, polling, not for time critical task */
uint8_t uart_rx() {
	TCNT0 = 0; // Reset timer
	TIFR = (1<<OCF0B);
	uint8_t data;

	while (PINB & (1<<PIN_UART)); // Waiting for start bit (low) to start timer
	TCCR0B = (1<<CS00);

	for (uint8_t c = 9; c; c--) { // Read 1 start bit (discard) and 8 data bits
		while (!( TIFR & (1<<OCF0B) ));
		TIFR = (1<<OCF0B);
		data = data >> 1;
		if (PINB & (1<<PIN_UART))
			data |= 0x80;
	}

	while (!( TIFR & (1<<OCF0B) )); // Since Tx and Rx share the same line
	TIFR = (1<<OCF0B);
	while (!( TIFR & (1<<OCF0B) )); // Wait for 2.5 stop bits to prevent conflict (sample point for 3rd stop bit)
	TIFR = (1<<OCF0B);
	while (!( TIFR & (1<<OCF0B) ));

	TCCR0B = (0<<CS00); // Stop timer to save energy
	return data;
}