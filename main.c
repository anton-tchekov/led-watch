#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdint.h>
#include "led.c"
#include "timer.c"
#include "piezo.c"

#define MODE_IDLE   0
#define MODE_CLOCK  1
#define MODE_UART   2
#define MODE_ALARM  3

static volatile uint8_t _mode;

static void mode_idle(void)
{
	_mode = MODE_IDLE;
	_beep = 0;
	_display = 0;
}

#include "button.c"
#include "font.c"
#include "clock.c"
#include "uart.c"
#include "alarm.c"

static uint8_t get_digit(void)
{
	return uart_rx() - '0';
}

static uint8_t parse_digits(void)
{
	uint8_t c, d;
	c = get_digit();
	d = get_digit();
	return c * 10 + d;
}

int main(void)
{
	led_init();
	button_init();
	timer_init();
	for(;;)
	{
next:
		if(_mode == MODE_UART)
		{
			/* Command Start */
			while(uart_rxc() != 'A')
			{
				if(_mode != MODE_UART)
				{
					goto next;
				}
			}

			/* Set Clock Command: */
			/* [WEEKDAY 0-6][HOUR 00-23][MINUTE 0-59] */
			_weekday = get_digit();
			_hours = parse_digits();
			_minutes = parse_digits();

			/* Set Alarms Command: */
			/* [ALARM COUNT 0-8] */
			/* N x [WEEKDAY 0-6][HOUR 00-23][MINUTE 0-59][ICON (25 Chars)] */
			_alarms_count = get_digit();
			uint8_t i, j;
			for(i = 0; i < _alarms_count; ++i)
			{
				_alarms[i].Weekday = get_digit();
				_alarms[i].Hours = parse_digits();
				_alarms[i].Minutes = parse_digits();
				_alarms[i].Icon = 0;
				for(j = 0; j < 25; ++j)
				{
					if(uart_rx() == '1')
					{
						_alarms[i].Icon |= (1UL << (uint32_t)j);
					}
				}
			}

			uart_tx_str_P(PSTR("ACK\n"));
			mode_idle();
		}
	}

	return 0;
}

ISR(TIMER0_COMPA_vect)
{
	if(_mode == MODE_CLOCK)
	{
		scroll_update();
	}

	timer_update();
	piezo_update();
	button_update();
	led_update();
	alarm_update();
}

static void mode_idle_button(uint8_t button)
{
	switch(button)
	{
		case 0:
			mode_clock();
			break;

		case 3:
			mode_uart();
			break;
	}
}

static void button_event(uint8_t button)
{
	switch(_mode)
	{
		case MODE_IDLE:
			mode_idle_button(button);
			break;

		case MODE_CLOCK:
		case MODE_ALARM:
			mode_idle();
			break;

		case MODE_UART:
			mode_uart_button();
			break;
	}
}
