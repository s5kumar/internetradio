
/*
 * Engine.h
 *
 *  Created on: 27-Jun-2021
 *      Author: anand
 */
#ifndef ENGINE_H
#define ENGINE_H
#include <gst/gst.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
using namespace std;
struct stationdata
{
	
	int station_num;
	string station_name;
	string station_url;
	string station_type;
};

class player
{
private:
	int current_volume;
	int current_station_num;
	int prev_station_num;
	bool current_state;
	bool current_mute;
	stationdata stData[100];
	int max_stations;
	GstElement *playbin;  /* Our one and only element */
	GMainLoop *main_loop;  /* GLib's Main Loop */
	GstBus *bus;
	void gstreamer_play(int);	
	void gstreamer_file_play(int);
	int findstationindex(int);
	bool file_play;

public:
	player();
	void loadconfig();
	void saveconfig();
	void autoplay();
	void play(int station_num, bool);
	void stop();
	void increasevol();
	void decreasevol();
	void toggleplaypause();
	void togglemute();
	void previousstation();
	void nextstation();
	void lastplayedstation();
	void handleerrors(GstBus* bus);

};
#endif