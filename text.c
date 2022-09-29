static uint8_t _text[30] = { 36, 17, 14, 21, 21, 24, 36, 0xff };
static uint8_t _text_offset;

static void text_update(void)
{
	static uint8_t _cnt;
	uint8_t *s, c, i, j, o, w, offset;

	if(++_cnt < 20)
	{
		return;
	}

	_cnt = 0;

	s = _text;
	i = 0;
	offset = 0;
	while((c = *s++) != 0xFF)
	{
		o = pgm_read_byte(&_map[c]);
		w = offset + (pgm_read_byte(&_map[c + 1]) - o);

		while(offset < w)
		{
			if(offset >= _text_offset && i < 5)
			{
				_display[i++] = pgm_read_byte(_font + o);
			}

			++o;
			++offset;
		}

		if(offset >= _text_offset && i < 5)
		{
			_display[i++] = 0;
		}

		++offset;
	}

	if(i < 5)
	{
		_text_offset = 0;
		return;
	}

	++_text_offset;
}

static void mode_text(void)
{
	_text_offset = 0;
	_mode = MODE_TEXT;
}
