static uint8_t _beep;

static void piezo_update(void)
{
	if(_beep)
	{
		uint8_t l = _ticks & 0x0F;
		if(l == 0)
		{
			PORTD |= (1 << 4);
		}
		else if(l == 8)
		{
			PORTD &= ~(1 << 4);
		}
	}
}
