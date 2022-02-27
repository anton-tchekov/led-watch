#define BUTTON_COUNT           4
#define BUTTON_PRESS_TICKS   750

static uint16_t button_ticks[BUTTON_COUNT];

static void button_event(uint8_t button);

static void button_init(void)
{
	PORTD |= (1 << 0) | (1 << 1) | (1 << 2) | (1 << 3);
}

static void button_update(void)
{
	uint8_t i, start = (_mode == MODE_UART) ? 2 : 0;
	for(i = start; i < BUTTON_COUNT; ++i)
	{
		if(!(PIND & (1 << i)))
		{
			if(button_ticks[i] < BUTTON_PRESS_TICKS)
			{
				++button_ticks[i];
			}
			else if(button_ticks[i] == BUTTON_PRESS_TICKS)
			{
				button_ticks[i] = BUTTON_PRESS_TICKS + 1;
				button_event(i);
			}
		}
		else
		{
			button_ticks[i] = 0;
		}
	}
}
