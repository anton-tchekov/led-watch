#define SECOND_TICKS 10800

static uint16_t
	_ticks;

static volatile uint8_t
	_seconds,
	_minutes,
	_hours,
	_weekday;

static void timer_init(void)
{
	TCCR0A |= (1 << WGM01);
	TCCR0B |= (1 << CS01) | (1 << CS00);
	OCR0A = 16;
	TIMSK |= (1 << OCIE0A);
	sei();
}

static void timer_update(void)
{
	if(++_ticks < SECOND_TICKS)
	{
		return;
	}

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
				if(++_weekday == 7)
				{
					_weekday = 0;
				}
			}
		}
	}
}
