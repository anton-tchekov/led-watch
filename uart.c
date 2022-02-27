#define UART_BAUD 9600
#define _BAUD         (((F_CPU / (UART_BAUD * 16UL))) - 1)

static void uart_tx_str_P(const char *s);

static void uart_init(void)
{
	/* Disable Pullups */
	PORTD &= ~((1 << 0) | (1 << 1));

	UCSRB = (1 << RXEN) | (1 << TXEN);
	UCSRC = (1 << UCSZ1) | (1 << UCSZ0);
	UBRRL = (uint8_t)(_BAUD & 0xFF);
	UBRRH = (uint8_t)((_BAUD >> 8) & 0xFF);

	uart_tx_str_P(PSTR("RDY\n"));
}

static char uart_rxc(void)
{
	if(!(UCSRA & (1 << RXC)))
	{
		return -1;
	}

	return UDR;
}

static char uart_rx(void)
{
	while(!(UCSRA & (1 << RXC))) ;
	return UDR;
}

static void uart_tx(char c)
{
	while(!(UCSRA & (1 << UDRE))) ;
	UDR = c;
}

static void uart_tx_str_P(const char *s)
{
	register char c;
	while((c = pgm_read_byte(s++)))
	{
		uart_tx(c);
	}
}

static void mode_uart(void)
{
	_display =
		(0b01000UL <<  0UL) |
		(0b11111UL <<  5UL) |
		(0b01000UL << 10UL) |
		(0b01011UL << 15UL) |
		(0b01010UL << 20UL);

	uart_init();
	_mode = MODE_UART;
}

static void mode_uart_button(void)
{
	UCSRB = 0;
	button_init();
	mode_idle();
}
