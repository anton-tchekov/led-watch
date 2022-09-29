#define SECOND_TICKS 1800

static uint8_t
	_subticks,
	_ticks;

static volatile uint8_t
	_seconds,
	_minutes,
	_hours;

static void timer_init(void)
{
	TCCR0A |= (1 << WGM01);
	TCCR0B |= (1 << CS01);
	OCR0A = 225 - 1;
	TIMSK |= (1 << OCIE0A);
	sei();
}

static void timer_update(void)
{
	if(++_subticks == 64)
	{
		_subticks = 0;
		tick();
		if(++_ticks == 128)
		{
			_ticks = 0;
			if(++_seconds == 60)
			{
				_seconds = 0;
				if(++_minutes == 60)
				{
					_minutes = 0;
					if(++_hours == 24)
					{
						_hours = 0;
					}
				}
			}
		}
	}
}
