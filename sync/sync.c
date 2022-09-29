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

typedef struct ALARM
{
	int Minute, Hour;
	unsigned char Icon[5];
} Alarm;

int main(int argc, char **argv)
{
	FILE *fp;
	int i, j, k, fd, hour, minute, second, num_alarms;
	Alarm alarms[4];
	char *port, *filename, buf[512];
	if(argc != 4)
	{
		fprintf(stderr, "Usage: ./sync port alarms-file scroll-text\n");
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

		q = p + 2;
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

		int h, m;
		h = (p[0] - '0') * 10 + (p[1] - '0');
		m = (q[0] - '0') * 10 + (q[1] - '0');

		if(h < 0 || h > 59 || m < 0 || m > 59)
		{
			printf("Invalid format: Hour/minute.\n");
			return 1;
		}

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
					alarms[num_alarms].Icon[i] |= (1 << j);
				}
				else if(buf[j] == '-')
				{
					alarms[num_alarms].Icon[i] &= ~(1 << j);
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

	unsigned char out[512], *p = out;

	*p++ = 'A';

	p += 3;

	*p++ = num_alarms;

	printf("Number of Alarms: %d\n", num_alarms);

	for(i = 0; i < num_alarms; ++i)
	{
		printf("Alarm %d: %02d:%02d\n", i, alarms[i].Hour, alarms[i].Minute);

		printf("Icon: +-----+\n");
		for(j = 0; j < 5; ++j)
		{
			printf("      |");
			for(k = 0; k < 5; ++k)
			{
				printf("%c", (alarms[i].Icon[j] & (1 << k)) ? '#' : ' ');
			}

			printf("|\n");
		}

		printf("      +-----+\n\n");

		*p++ = alarms[i].Hour;
		*p++ = alarms[i].Minute;
		memcpy(p, alarms[i].Icon, 5);
		p += 5;
	}

	char *s, c;
	s = argv[3];
	*p++ = 10 + 'Z' - 65 + 1;
	while((c = *s++))
	{
		if(isdigit(c))
		{
			*p++ = c - '0';
		}
		else if(c == ' ')
		{
			*p++ = 10 + 'Z' - 65 + 1;
		}
		else if(c >= 'A' && c <= 'Z')
		{
			*p++ = 10 + c - 65;
		}
	}

	*p++ = 10 + 'Z' - 65 + 1;
	*p++ = 0xFF;

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

	printf("\nTransfering data ...\n");

	time_t now = time(NULL) + 60 * 60;
	struct tm *tm_struct = localtime(&now);
	hour = tm_struct->tm_hour;
	minute = tm_struct->tm_min;
	second = tm_struct->tm_sec;

	/* Correct Time Zone */
	if(++hour == 24)
	{
		hour = 0;
	}

	out[1] = hour;
	out[2] = minute;
	out[3] = second;

	write(fd, out, p - out);

	printf("\nCurrent Time: %02d:%02d\n", hour, minute);

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

