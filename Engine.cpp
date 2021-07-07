#include <iostream>
#include <fstream>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

#include "Engine.h"
using namespace std;
#define RADIOCHANNEL_LIST "/etc/radio/Radiodata.dat"
#define AUTOSAVE_PREF "/etc/radio/save.dat"

player::player() : current_volume(10), current_station_num(0), prev_station_num(0), max_stations(0), current_mute(false), current_state(true)
{
	char data[200];
	ifstream  infile;
	infile.open(RADIOCHANNEL_LIST);

	while (!infile.eof())
	{
		infile.getline(data, 200, '\n');
		string tokens[4];
		char* token = strtok(data, ",");

		// Keep printing tokens while one of the
		// delimiters present in str[].
		int j = 0;
		bool bfound = false;
		while (token != NULL)
		{
			//printf("%s\n", token);
			tokens[j++] = token;
			token = strtok(NULL, ",");
			bfound = true;
		}
		if (bfound)
		{
			stData[max_stations].station_type = tokens[0];
			stData[max_stations].station_num = atoi(tokens[1].c_str());
			stData[max_stations].station_name = tokens[2];
			stData[max_stations].station_url = tokens[3];
			max_stations++;
			bfound = false;
		}

	}
	loadconfig();

}
void player::loadconfig()
{
	ifstream infile(AUTOSAVE_PREF);
	infile >> current_station_num;
	infile >> current_volume;
	
}

void player::saveconfig()
{
	ofstream outfile(AUTOSAVE_PREF);
	outfile << current_station_num << endl;
	outfile << current_volume << endl;
}

void player::handleerrors(GstBus* bus)
{
	GstMessage* msg;
	GstStateChangeReturn ret;
	gboolean terminate = FALSE;

	do {
		msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE,
			GstMessageType(GST_MESSAGE_STATE_CHANGED | GST_MESSAGE_ERROR | GST_MESSAGE_EOS)
			);

		if (msg != NULL) {
			GError* err;
			gchar* debug_info;

			switch (GST_MESSAGE_TYPE(msg)) {
			case GST_MESSAGE_ERROR:
				gst_message_parse_error(msg, &err, &debug_info);
				system("echo \"station not found\" | festival --tts --language english ");
				/*g_printerr("error received from element %s:%s\n",
					GST_OBJECT_NAME(msg->src), err->message);
				g_printerr("debugging information: %s\n",
					debug_info ? debug_info : "none");*/
				g_clear_error(&err);
				g_free(debug_info);
				terminate = TRUE;
				break;
			case GST_MESSAGE_EOS:
				system("echo I am not able to play the station for some reason | festival --tts --language english ");
				//g_print("end of stream reached\n");
				terminate = TRUE;
				break;
		
			default:
				g_printerr("unexpected message received\n");
				break;
			}
			gst_message_unref(msg);
		}
	} while (!terminate);

}


void player::gstreamer_play(int index)
{
	GstStateChangeReturn ret;
	gst_init (NULL, NULL);
	playbin = gst_element_factory_make ("playbin", "playbin");

	file_play=false;
	if (!playbin) {
		g_printerr ("Not all elements could be created.\n");
		return;
	}

	/* Set the URI to play */
	string str = stData[index].station_url;
	char * writable = new char[str.size() + 1];
	std::copy(str.begin(), str.end(), writable);
	writable[str.size()] = '\0'; // don't forget the terminating 0
	
	g_object_set (playbin, "uri", writable, NULL);
	double vol = (double)(current_volume)/10;

	g_object_set (playbin, "volume", vol, NULL);

	/* Add a bus watch, so we get notified when a message arrives */
	bus = gst_element_get_bus (playbin);
	//gst_bus_add_watch (bus,  player::bus_callback, NULL);

	/* Start playing */
	ret = gst_element_set_state(playbin, GST_STATE_PLAYING);//currently no error handling...just hoping it plays...cant do much
	if (ret == GST_STATE_CHANGE_FAILURE) {
		//g_printerr ("Unable to set the pipeline to the playing state.\n");
		system("echo I am not able to play the station for some reason | festival --tts --language english ");
		//gst_object_unref (playbin);
		//return ;
	}
	handleerrors(bus);
	/* Create a GLib Main Loop and set it to run */
	//main_loop = g_main_loop_new (NULL, FALSE);
	//g_main_loop_run (main_loop);

	/* Free resources */
	//g_main_loop_unref (main_loop);

	gst_object_unref (bus);
	gst_element_set_state (playbin, GST_STATE_NULL);
	gst_object_unref (playbin);	

}
void player::gstreamer_file_play(int index)
{
	
	string str = "curl --referer http://www.tamilradios.com ";
	str.append(stData[index].station_url);

	str.append(" > /tmp/stream.mp3 &");

	system(str.c_str());
	sleep(2);
	
	GstStateChangeReturn ret;
	gst_init (NULL, NULL);
	playbin = gst_element_factory_make ("playbin", "playbin");

	g_printerr("I am here");
	if (!playbin) {
		g_printerr ("Not all elements could be created.\n");
		return;
	}

	file_play=true;
	
	g_object_set (playbin, "uri", "file:///tmp/stream.mp3", NULL);
	double vol = (double)(current_volume)/10;

	g_object_set (playbin, "volume", vol, NULL);

	/* Add a bus watch, so we get notified when a message arrives */
	bus = gst_element_get_bus (playbin);
	//gst_bus_add_watch (bus,  NULL, &data);

	/* Start playing */
	ret = gst_element_set_state(playbin, GST_STATE_PLAYING);//currently no error handling...just hoping it plays...cant do much
	if (ret == GST_STATE_CHANGE_FAILURE) {
		//g_printerr ("Unable to set the pipeline to the playing state.\n");
		//gst_object_unref (playbin);
		system("echo I am not able to play the station for some reason | festival --tts --language english ");
		return ;
	}

	handleerrors(bus);
	/* Create a GLib Main Loop and set it to run */
	//main_loop = g_main_loop_new (NULL, FALSE);
	//g_main_loop_run (main_loop);

	/* Free resources */
	//g_main_loop_unref (main_loop);

	gst_object_unref (bus);
	gst_element_set_state (playbin, GST_STATE_NULL);
	gst_object_unref (playbin);	

}
void player::autoplay()
{
	int station_index = findstationindex(current_station_num);

	if(stData[station_index].station_type=="L")
		gstreamer_play(station_index);
	else if (stData[station_index].station_type=="C")
		gstreamer_file_play(station_index);

}

int player::findstationindex(int station_num)
{
	int station_index = 255;
	for (int i = 0; i < max_stations; i++)
	{
		if (stData[i].station_num == station_num)
		{
			station_index = i;
			break;
		}
	}
	return station_index;
}

void player::play(int station_num, bool autoplaymode = false)
{
	int station_index = findstationindex(station_num);

	if (station_index == 255)
		return;
	if (current_station_num != station_num)
	{
		stop();
	}
	/*else if (autoplaymode == false)
		return;
		*/
	prev_station_num = current_station_num;
	current_station_num = station_num;
	saveconfig();

	string speakcommand("echo ");
	speakcommand.append(stData[station_index].station_name);
	speakcommand.append(" | festival --tts --language english");
	system(speakcommand.c_str());
	
	
	if(stData[station_index].station_type=="L")
	{
		/* Set the URI to play */
		string str = stData[station_index].station_url;
		char * writable = new char[str.size() + 1];
		std::copy(str.begin(), str.end(), writable);
		writable[str.size()] = '\0'; // don't forget the terminating 0
		
		g_object_set (playbin, "uri", writable, NULL);
		double vol = (double)(current_volume)/10;

		g_object_set (playbin, "volume", vol, NULL);
	
		gst_element_set_state (playbin, GST_STATE_PLAYING);
		file_play = false;
	}
	else if (stData[station_index].station_type=="C")
	{
		string str = "curl --referer http://www.tamilradios.com ";
		str.append(stData[station_index].station_url);
		str.append(" > /tmp/stream.mp3 &");
		system(str.c_str());
		sleep(2);
		g_object_set (playbin, "uri", "file:///tmp/stream.mp3", NULL);
		double vol = (double)(current_volume)/10;

		g_object_set (playbin, "volume", vol, NULL);
	
		gst_element_set_state (playbin, GST_STATE_PLAYING); //currently no error handling...just hoping it plays...cant do much
		file_play=true;
	}

	current_state = true;
	
}
void player::stop()
{
	if(file_play)
	{
		system("killall curl");
	}
	gst_element_set_state (playbin, GST_STATE_NULL);
	current_state = false;
}

void player::toggleplaypause()
{
	if(current_state)
		stop();
	else
		play(current_station_num,true);
}

void player::togglemute()
{
	if(current_mute)
	{
		double vol = (double)(current_volume)/10;

		g_object_set (playbin, "volume", vol, NULL);
		current_mute = false;
	}
	else
	{
		double vol = 0.0;

		g_object_set (playbin, "volume", vol, NULL);
		current_mute = true;
	}
}

void player::increasevol()
{
	current_volume+=5;
	if(current_volume>=100)
	{
		current_volume=100;
	}
	
	double vol = (double)(current_volume)/10;

	g_object_set (playbin, "volume", vol, NULL);
	saveconfig();
}

void player::decreasevol()
{
	current_volume-=5;
	if(current_volume<=0)
		current_volume = 0;
	
	double vol = (double)(current_volume)/10;

	g_object_set (playbin, "volume", vol, NULL);
	saveconfig();
}


void player::previousstation()
{
	int station_index = findstationindex(current_station_num);
	if (station_index == 0)
		station_index = max_stations;
	station_index--;
	cout << station_index << " " << stData[station_index].station_num;

	play(stData[station_index].station_num);

}
void player::nextstation()
{
	int station_index = findstationindex(current_station_num);
	if (station_index == max_stations - 1)
		station_index = 0;
	else
		station_index++;
	cout << station_index << " " << stData[station_index].station_num;

	play(stData[station_index].station_num);

}
void player::lastplayedstation()
{
	play(prev_station_num);
}