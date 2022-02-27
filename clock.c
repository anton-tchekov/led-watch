static uint8_t _cur, _buf[4];

static void scroll_update(void)
{
	static uint16_t _cnt;
	if(++_cnt < 3000)
	{
		return;
	}

	_cnt = 0;

	if(_cur < 4)
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

		_display =
			((uint32_t)(lines[0]) <<  0UL) |
			((uint32_t)(lines[1]) <<  5UL) |
			((uint32_t)(lines[2]) << 10UL) |
			((uint32_t)(lines[3]) << 15UL) |
			((uint32_t)(lines[4]) << 20UL);
	}

	if(++_cur == 5)
	{
		mode_idle();
	}
}

static void mode_clock(void)
{
	_buf[0] = _hours / 10;
	_buf[1] = _hours % 10;
	_buf[2] = _minutes / 10;
	_buf[3] = _minutes % 10;
	_cur = 0;
	_mode = MODE_CLOCK;
}
