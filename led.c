#define BRIGHTNESS_FACTOR 10

static uint8_t _display[5];

static const uint8_t display_clear[5] PROGMEM =
{
	0x00, 0x00, 0x00, 0x00, 0x00
};

static void led_init(void)
{
	/* LED Pins output */
	DDRB = 0xFF;
	DDRD |= (1 << 6) | (1 << 5) | (1 << 4);
}

static void display_set(const uint8_t *value)
{
	_display[0] = value[0];
	_display[1] = value[1];
	_display[2] = value[2];
	_display[3] = value[3];
	_display[4] = value[4];
}

static void display_set_P(const uint8_t *value)
{
	_display[0] = pgm_read_byte(value + 0);
	_display[1] = pgm_read_byte(value + 1);
	_display[2] = pgm_read_byte(value + 2);
	_display[3] = pgm_read_byte(value + 3);
	_display[4] = pgm_read_byte(value + 4);
}

static void _led_set(uint8_t line)
{
	uint8_t j = 0;

	/* Disable all lines */
	PORTB &= ~((1 << 7) |
			(1 << 6) |
			(1 << 5) |
			(1 << 4) |
			(1 << 3));

	/* Set Current Line */
	PORTB |= (1 << (line + 3));

	/* LEDs On/Off */
	PORTB |= (1 << 2) |
			(1 << 1) |
			(1 << 0);

	PORTD |= (1 << 6) |
			(1 << 5);

	if(_display[line] & (1 << j))
	{
		PORTB &= ~(1 << 2);
	}

	++j;
	if(_display[line] & (1 << j))
	{
		PORTB &= ~(1 << 1);
	}

	++j;
	if(_display[line] & (1 << j))
	{
		PORTB &= ~(1 << 0);
	}

	++j;
	if(_display[line] & (1 << j))
	{
		PORTD &= ~(1 << 6);
	}

	++j;
	if(_display[line] & (1 << j))
	{
		PORTD &= ~(1 << 5);
	}
}

static void _led_clear_bright(void)
{
	PORTB |= (1 << 2);
	PORTB |= (1 << 0);
	PORTD |= (1 << 5);
}

static void led_update(void)
{
	static uint8_t sd, i;

	if(sd == 0)
	{
		_led_set(i);
	}
	else if(sd == 1)
	{
		/* Compensate for brighter leds */
		if(i == 0 || i == 2 || i == 4)
		{
			_led_clear_bright();
		}
	}

	if(++sd == BRIGHTNESS_FACTOR)
	{
		sd = 0;
		if(++i == 5)
		{
			i = 0;
		}
	}
}
