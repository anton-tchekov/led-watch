#define BRIGHTNESS_FACTOR 10

static uint32_t _display;

static void led_init(void)
{
	/* LED Pins output */
	DDRB = 0xFF;
	DDRD |= (1 << 6) | (1 << 5) | (1 << 4);
}

static void _led_set(uint8_t line)
{
	uint8_t j = 5 * line;

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

	if(_display & (1UL << j))
	{
		PORTB &= ~(1 << 2);
	}

	++j;
	if(_display & (1UL << j))
	{
		PORTB &= ~(1 << 1);
	}

	++j;
	if(_display & (1UL << j))
	{
		PORTB &= ~(1 << 0);
	}

	++j;
	if(_display & (1UL << j))
	{
		PORTD &= ~(1 << 6);
	}

	++j;
	if(_display & (1UL << j))
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
