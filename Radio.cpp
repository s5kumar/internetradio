

//============================================================================
// Name        : Radio.cpp
// Author      :
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/input.h>
#include <fcntl.h>
#include <math.h>
#include <pthread.h>
#include <time.h>

#include "Engine.h"

#define RASPBERRY_PI4_REMOTE 1
#ifdef BEAGLE_REMOTE_KEYBOARD
#define KEYFILE "/dev/input/event1"
#define PLAYFILE "/dev/input/event3"
#define CONTROLFILE "/dev/input/event4"
#endif

#ifdef RASPBERRY_PI4_REMOTE
#define KEYFILE "/dev/input/event0"
#define PLAYFILE "/dev/input/event2"
#define CONTROLFILE "/dev/input/event3"
#endif

#define PRESS_DOWN 1
#define CODE_LEFT 105
#define CODE_LLEFT 109
#define CODE_RRIGHT 104
#define CODE_RIGHT 106
#define CONT_LEFT 165
#define CONT_LLEFT 168
#define CONT_RIGHT 163
#define CONT_RRIGHT 208
#define PLAYPAUSE 164
#define STOP 116
#define VOLUP 115
#define VOLDOWN 114
#define MUTE 113
#define NEWPLAYPAUSE 127
#define BACK 158


using namespace std;

//just using don't care global variables
player pl;
int station_num;
int device_available = 0;
bool timer_active=false;
int sel_num = 0;

//simple logic to calculate remote keys
void* timer_dummy_func(void* arg)
{
	cout<<"Into timer thread"<<endl;
	sleep(3);
	timer_active = false;
	cout<<sel_num;
	pl.play(sel_num,false);
	sel_num = 0;
}

//handler function for handling selection keys from remote
void* keyfunc(void* arg)
{
	bool succ = false;
	pthread_t pkeytimerid;
	bool timeout=false;
	int fd;
	struct input_event ie;
	pthread_detach(pthread_self());

	while(!succ)
	{
		if ((fd = open(KEYFILE, O_RDONLY)) == -1) {
			perror("opening key device");
			sleep(5);
			}
		else
		{
			succ=true;
			device_available+=1;

		}

	}

	while (read(fd, &ie, sizeof(struct input_event)))
	{

		if (ie.value == PRESS_DOWN)
		{
			if (ie.code == CODE_LEFT || ie.code == CODE_LLEFT)
				pl.previousstation();
			else if (ie.code == CODE_RIGHT || ie.code == CODE_RRIGHT)
				pl.nextstation();
			else if (ie.code == NEWPLAYPAUSE)
				pl.toggleplaypause();
			else if (ie.code > 0 || ie.code <10)
			{

				cout<<"code: "<<ie.code<<" "<<timer_active<<endl;
				if(!timer_active)
				{
					timer_active = true;
					sel_num=0;
					pthread_create(&pkeytimerid, NULL, &timer_dummy_func, &pl);
					cout<<"timer_created";
				}
				//only calculate the selected channel value...after 3 secs automatically the station number shall be selected
				sel_num = 10*sel_num + (ie.code - 1);
				
			}

		}
	}
    // exit the current thread
    pthread_exit(NULL);
}

//handler function for volume keys
void* playfunc(void* arg)
{
	bool succ = false;

	int fd;
	struct input_event ie;
	pthread_detach(pthread_self());

	while(!succ)
	{
		if ((fd = open(PLAYFILE, O_RDONLY)) == -1) {
			perror("opening play device");
			sleep(5);
			}
		else
		{
			succ=true;
			device_available+=1;
		}

	}

	while (read(fd, &ie, sizeof(struct input_event)))
	{
		cout<<"code ="<<ie.code<<" value="<<ie.value<<endl;

		if (ie.value == PRESS_DOWN)
		{
			if (ie.code == CONT_LEFT || ie.code == CONT_LLEFT)
				pl.previousstation();
			else if (ie.code == CONT_RIGHT || ie.code == CONT_RRIGHT)
				pl.nextstation();
			else if (ie.code == BACK)
				pl.lastplayedstation();
			else if(ie.code == PLAYPAUSE)
				pl.toggleplaypause();
			else if(ie.code == VOLDOWN)
				pl.decreasevol();
			else if(ie.code == VOLUP)
				pl.increasevol();
			else if(ie.code == MUTE)
				pl.togglemute();
		}
	}

    // exit the current thread
    pthread_exit(NULL);
}

#ifdef ENABLE_CONTROL_HANDLING
//handler function for stop key
void* controlfunc(void* arg)
{
	bool succ = false;

	int fd;
	struct input_event ie;
	pthread_detach(pthread_self());

	while(!succ)
	{
		if ((fd = open(CONTROLFILE, O_RDONLY)) == -1) {
			perror("opening play device");
			sleep(5);
			}
		else
		{
			succ=true;
			device_available+=1;
		}

	}
	while (read(fd, &ie, sizeof(struct input_event)))
	{
		cout<<"code ="<<ie.code<<" value="<<ie.value<<endl;

		if (ie.value == PRESS_DOWN)
		{
			if(ie.code == STOP)
				pl.toggleplaypause(); //alternatively >> system("sudo shutdown now())"); >> but system cannot be woken up without power on
		}
	}

    // exit the current thread
    pthread_exit(NULL);
}
#endif

int main() {

	pthread_t pkeytid, pplaytid;
	// Creating a new thread
	pthread_create(&pkeytid, NULL, &keyfunc, &pl);
	pthread_create(&pplaytid, NULL, &playfunc, &pl);


#ifdef ENABLE_CONTROL_HANDLING
	pthread_t pcontroltid;
	pthread_create(&pcontroltid, NULL, &controlfunc, &pl);
	while(device_available!=3)
#else
	while (device_available != 2)
#endif
	{
		sleep(1);//wait until remote is available
	}

	pl.autoplay();

	pthread_join(pkeytid, NULL);
	pthread_join(pplaytid, NULL);
#ifdef ENABLE_CONTROL_HANDLING
	pthread_join(pcontroltid, NULL);
#endif
	pthread_exit(NULL);

	return 0;
}

