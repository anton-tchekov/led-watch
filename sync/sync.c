#include <stdio.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <ctype.h>

static int set_interface_attribs(int fd, int speed, int parity)
{
	struct termios tty;
	if(tcgetattr(fd, &tty) != 0)
	{
		fprintf(stderr, "(tcgetattr) Error %d\n", errno);
		return -1;
	}

	cfsetospeed(&tty, speed);
	cfsetispeed(&tty, speed);
	tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;
	tty.c_iflag &= ~IGNBRK;
	tty.c_lflag = 0;
	tty.c_oflag = 0;
	tty.c_cc[VMIN]  = 0;
	tty.c_cc[VTIME] = 5;
	tty.c_iflag &= ~(IXON | IXOFF | IXANY);
	tty.c_cflag |= (CLOCAL | CREAD);
	tty.c_cflag &= ~(PARENB | PARODD);
	tty.c_cflag |= parity;
	tty.c_cflag &= ~CSTOPB;
	tty.c_cflag &= ~CRTSCTS;
	if(tcsetattr(fd, TCSANOW, &tty) != 0)
	{
		fprintf(stderr, "(tcsetattr) Error %d\n", errno);
		return -1;
	}

	return 0;
}

static int set_blocking(int fd, int should_block)
{
	struct termios tty;
	memset(&tty, 0, sizeof tty);
	if(tcgetattr(fd, &tty) != 0)
	{
		fprintf(stderr, "(tcgetattr) Error %d\n", errno);
		return -1;
	}

	tty.c_cc[VMIN] = !!should_block;
	tty.c_cc[VTIME] = 5;
	if(tcsetattr(fd, TCSANOW, &tty) != 0)
	{
		fprintf(stderr, "(tcsetattr) Error %d\n", errno);
		return -1;
	}

	return 0;
}

static int init_fd(char *port)
{
	int fd = open(port, O_RDWR | O_NOCTTY | O_SYNC);
	if(fd < 0)
	{
		printf("(open) Error %d: %s\n", errno, strerror(errno));
		return -1;
	}

	set_interface_attribs(fd, B9600, 0);
	set_blocking(fd, 1);
	return fd;
}

const char *_weekdays[7] =
{
	"Sunday",
	"Monday",
	"Tuesday",
	"Wednesday",
	"Thursday",
	"Friday",
	"Saturday"
};

typedef struct ALARM
{
	int Minute, Hour, Weekday;
	char Icon[32];
} Alarm;

int main(int argc, char **argv)
{
	FILE *fp;
	int i, j, k, fd, day, hour, minute, num_alarms;
	Alarm alarms[6];
	char *port, *filename, buf[512];
	if(argc != 3)
	{
		fprintf(stderr, "Usage: ./sync port alarms-file\n");
		return 1;
	}

	filename = argv[2];
	if(!(fp = fopen(filename, "r")))
	{
		fprintf(stderr, "Failed to open \"%s\"\n", filename);
		return 1;
	}

	num_alarms = 0;
	while(fgets(buf, sizeof(buf), fp))
	{
		if(isspace(buf[0]))
		{
			continue;
		}

		char *p, *q;
		p = buf;
		while(*p != ',')
		{
			if(*p == '\0')
			{
				printf("Invalid format: Comma not found.\n");
				return 1;
			}

			++p;
		}

		*p = '\0';
		for(i = 0; i < 7; ++i)
		{
			if(strcmp(buf, _weekdays[i]) == 0)
			{
				break;
			}
		}

		p += 2;
		q = p;
		while(*q != ':')
		{
			if(*q == '\0')
			{
				printf("Invalid format: Colon not found.\n");
				return 1;
			}

			++q;
		}

		*q = '\0';
		++q;

		if(i == 7)
		{
			printf("Invalid format: Weekday not found.\n");
			return 1;
		}

		int h, m;
		h = (p[0] - '0') * 10 + (p[1] - '0');
		m = (q[0] - '0') * 10 + (q[1] - '0');

		if(h < 0 || h > 59 || m < 0 || m > 59)
		{
			printf("Invalid format: Hour/minute.\n");
			return 1;
		}

		alarms[num_alarms].Weekday = i;
		alarms[num_alarms].Hour = h;
		alarms[num_alarms].Minute = m;

		for(i = 0; i < 5; ++i)
		{
			if(!fgets(buf, sizeof(buf), fp))
			{
				printf("Invalid format: Icon line %d missing.\n", i);
				return 1;
			}

			for(j = 0; j < 5; ++j)
			{
				if(buf[j] == '#')
				{
					alarms[num_alarms].Icon[5 * i + j] = '1';
				}
				else if(buf[j] == '-')
				{
					alarms[num_alarms].Icon[5 * i + j] = '0';
				}
				else
				{
					printf("Invalid format: Icon line %d:%d.\n", i, j);
					return 1;
				}
			}

			alarms[num_alarms].Icon[25] = '\0';
		}

		++num_alarms;
	}

	fclose(fp);

	time_t now = time(NULL);
	struct tm *tm_struct = localtime(&now);
	hour = tm_struct->tm_hour;
	minute = tm_struct->tm_min;
	day = tm_struct->tm_wday;

	/* Correct Time Zone */
	if(++hour == 24)
	{
		hour = 0;
	}

	char *p = buf;

	*p++ = 'A';

	*p++ = day + '0';
	*p++ = (hour / 10) + '0';
	*p++ = (hour % 10) + '0';
	*p++ = (minute / 10) + '0';
	*p++ = (minute % 10) + '0';
	*p++ = num_alarms + '0';

	printf("Number of Alarms: %d\n", num_alarms);

	for(i = 0; i < num_alarms; ++i)
	{
		printf("Alarm %d: %s, %02d:%02d\n", i, _weekdays[alarms[i].Weekday], alarms[i].Hour, alarms[i].Minute);

		printf("Icon: +-----+\n");
		for(j = 0; j < 5; ++j)
		{
			printf("      |");
			for(k = 0; k < 5; ++k)
			{
				printf("%c", alarms[i].Icon[j * 5 + k] == '1' ? '#' : ' ');
			}

			printf("|\n");
		}

		printf("      +-----+\n\n");

		*p++ = alarms[i].Weekday + '0';
		*p++ = (alarms[i].Hour / 10) + '0';
		*p++ = (alarms[i].Hour % 10) + '0';
		*p++ = (alarms[i].Minute / 10) + '0';
		*p++ = (alarms[i].Minute % 10) + '0';
		strcpy(p, alarms[i].Icon);
		p += 25;
	}

	*p = '\0';


	port = argv[1];
	if((fd = init_fd(port)) < 0)
	{
		fprintf(stderr, "Failed to open port \"%s\"\n", argv[1]);
		return 1;
	}

	printf("> Press [Button 4] to establish connection to LED Watch\n");
	printf(
		"Display: +-----+\n"
		"         | #   |\n"
		"         |#####|\n"
		"         | #   |\n"
		"         | # ##|\n"
		"         | # # |\n"
		"         +-----+\n\n");

	for(;;)
	{
		int n = 0;
		char rb[256];
		if((n = read(fd, rb + n, sizeof(rb))) < 0)
		{
			printf("Error Reading\n");
			return 1;
		}

		rb[n] = '\0';

		if(strcmp(rb, "RDY\n") == 0)
		{
			printf("LED_WATCH CONNECTED!\n");
			break;
		}
	}

	printf("\nCurrent Time: %s, %02d:%02d\n", _weekdays[day], hour, minute);
	printf("\nTransfering data ...\n");

	write(fd, buf, p - buf);

	for(;;)
	{
		int n;
		char rb[256];
		if((n = read(fd, rb, sizeof(rb))) < 0)
		{
			printf("Error Reading\n");
			return 1;
		}

		if(strcmp(rb, "ACK\n") == 0)
		{
			printf("Transfer complete!\n");
			break;
		}
	}

	close(fd);
	return 0;
}

