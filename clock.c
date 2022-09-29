static uint8_t _cur, _buf[6];

static void scroll_update(void)
{
	static uint8_t _cnt;
	if(++_cnt < 40)
	{
		return;
	}

	_cnt = 0;
	if(_cur < 6)
	{
		uint8_t lines[5], i, o, w, v, j, s = 0;
		v = _buf[_cur];
		o = pgm_read_byte(&_map[v]);
		w = pgm_read_byte(&_map[v + 1]) - o;
		for(i = 0; i < 5; ++i)
		{
			lines[i] = 0;
		}

		if((_cur & 1) == 1)
		{
			s = 5 - w;
		}

		for(i = 0; i < w; ++i)
		{
			uint8_t r = pgm_read_byte(_font + o + i);
			for(j = 0; j < 5; ++j)
			{
				if(r & (1 << j))
				{
					lines[4 - j] |= (1 << (s + i));
				}
			}
		}

		display_set(lines);
	}
	else
	{
		mode_idle();
	}

	++_cur;
}

static void mode_clock(void)
{
	_buf[0] = _hours / 10;
	_buf[1] = _hours % 10;
	_buf[2] = _minutes / 10;
	_buf[3] = _minutes % 10;
	_buf[4] = _seconds / 10;
	_buf[5] = _seconds % 10;
	_cur = 0;
	_mode = MODE_CLOCK;
}
