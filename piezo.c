static uint8_t _beep;

static void piezo_update(void)
{
	if(_beep)
	{
		uint8_t l = (_subticks >> 3) & 1;
		if(l)
		{
			PORTD |= (1 << 4);
		}
		else
		{
			PORTD &= ~(1 << 4);
		}
	}
}
