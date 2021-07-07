#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <linux/input.h>
#include <fcntl.h>
#define PRESS_DOWN 1
#define RASPBERRY_PI4_REMOTE 1
#ifdef BEAGLE_REMOTE_KEYBOARD
#define CONTROLFILE "/dev/input/event4"
#endif

#ifdef RASPBERRY_PI4_REMOTE
#define CONTROLFILE "/dev/input/event3"
#endif

#define STOP 116

using namespace std;


int main()
{
	bool succ = false;
	bool start = false;

	int fd;
	struct input_event ie;
	while (!succ)
	{
		if ((fd = open(CONTROLFILE, O_RDONLY)) == -1) {
			perror("opening play device");
			sleep(5);
		}
		else
		{
			succ = true;
		}

	}
	while (read(fd, &ie, sizeof(struct input_event)))
	{
		cout << "code =" << ie.code << " value=" << ie.value << endl;

		if (ie.value == PRESS_DOWN)
		{
			if (ie.code == STOP)
			{
				if (start)
				{
					start = false;
					system("sudo systemctl stop radio");
				}
				else
				{
					start = true;
					system("sudo systemctl start radio");
				}
			}

		}
	}

	return 0;
}