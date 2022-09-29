#include <avr/io.h>
#include <util/delay.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>
#include <string.h>
#include <stdint.h>

static void tick(void);

#include "led.c"
#include "timer.c"
#include "piezo.c"

#define MODE_IDLE   0
#define MODE_UART   1

#define MODE_CLOCK  2
#define MODE_ALARM  3
#define MODE_RANDOM 4
#define MODE_TEXT   5

static volatile uint8_t _mode;

static void mode_idle(void)
{
	_mode = MODE_IDLE;
	_beep = 0;
	display_set_P(display_clear);
}

#include "button.c"
#include "font.c"
#include "clock.c"
#include "uart.c"
#include "alarm.c"
#include "random.c"
#include "text.c"

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
			uint8_t i, j;

			/* Command Start */
			while(uart_rxc() != 'A')
			{
				if(_mode != MODE_UART)
				{
					goto next;
				}
			}

			/* Set Clock Command: */
			/* [WEEKDAY 0-6][HOUR 00-23][MINUTE 0-59][SECOND 0-59] */
			_hours = uart_rx();
			_minutes = uart_rx();
			_seconds = uart_rx();

			/* Set Alarms Command: */
			/* [ALARM COUNT 0-8] */
			/* N x [WEEKDAY 0-6][HOUR 00-23][MINUTE 0-59][ICON 5 Bytes] */
			_alarms_count = uart_rx();
			for(i = 0; i < _alarms_count; ++i)
			{
				_alarms[i].Hours = uart_rx();
				_alarms[i].Minutes = uart_rx();
				for(j = 0; j < 5; ++j)
				{
					_alarms[i].Icon[j] = uart_rx();
				}
			}

			j = 0;
			do
			{
				i = uart_rx();
				_text[j++] = i;
			}
			while(i != 0xFF);

			uart_tx_str_P(PSTR("ACK\n"));
			mode_uart_button();
		}
	}

	return 0;
}

ISR(TIMER0_COMPA_vect)
{
	timer_update();
	piezo_update();
	led_update();
}

static void tick(void)
{
	if(_mode == MODE_CLOCK)
	{
		scroll_update();
	}
	else if(_mode == MODE_TEXT)
	{
		text_update();
	}

	button_update();
	alarm_update();
}

static void mode_idle_button(uint8_t button)
{
	switch(button)
	{
		case 0:
			mode_clock();
			break;

		case 1:
			mode_random();
			break;

		case 2:
			mode_text();
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

		case MODE_UART:
			mode_uart_button();
			break;

		case MODE_CLOCK:
		case MODE_ALARM:
		case MODE_RANDOM:
		case MODE_TEXT:
			mode_idle();
			break;
	}
}
