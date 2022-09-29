typedef struct ALARM
{
	uint8_t Minutes, Hours;
	uint8_t Icon[5];
} Alarm;

static volatile uint8_t _alarms_count;
static volatile Alarm _alarms[4], *_alarm_cur;

#define DELAY_SHORT   15
#define DELAY_LONG   224

static void alarm_update(void)
{
	if(_mode == MODE_ALARM)
	{
		static uint8_t delay = DELAY_SHORT, cnt = 0;
		static uint8_t i;

		if(++cnt == delay)
		{
			cnt = 0;

			if((i & 1) == 1)
			{
				_beep = 1;
				display_set(_alarm_cur->Icon);
			}
			else
			{
				_beep = 0;
				display_set_P(display_clear);
			}

			if(i == 7)
			{
				delay = DELAY_LONG;
				_beep = 0;
			}

			if(++i == 9)
			{
				delay = DELAY_SHORT;
				i = 0;
			}
		}
	}
	else if(_seconds == 0 && _ticks == 0)
	{
		volatile Alarm *a = &_alarms[_alarms_count - 1];
		while(a >= _alarms)
		{
			if(a->Minutes == _minutes &&
				a->Hours == _hours)
			{
				_alarm_cur = a;
				_mode = MODE_ALARM;
				break;
			}

			--a;
		}
	}
}
