// jr3_sensor_udp_streaming_server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <crtdbg.h>
#include <stdio.h>
#include <iostream>
#include <conio.h>
#include <ctime>

#include "config.h"
#include "jr3pci_ft.h"
#include "data_transmission.h"

#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

//Timer Constants
#define TIME_PERIOD 1
#define TIME_RESOLUTION 1

using namespace std;

#pragma pack(1)

// Global variables
data_transmission _transmission;
bool _two_sensors;
short _processor_number;
short _filter_address;
double _scale[6];
double _fm[12];

//Timer Vars
TIMECAPS tc;
UINT wTimerRes;
LARGE_INTEGER freq;

//Timer callback function, frequency set by define TIME_RESOLUTION
void CALLBACK timercbk(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2){
	// --- Read f/t data
	struct force_array ft0;
	struct force_array ft1;
	short num_doubles_ss = 6;

	ft0 = read_ftdata(_filter_address, _processor_number, 1);
	_fm[0] = _scale[0] * ft0.fx;
	_fm[1] = _scale[1] * ft0.fy;
	_fm[2] = _scale[2] * ft0.fz;
	_fm[3] = _scale[3] * ft0.mx;
	_fm[4] = _scale[4] * ft0.my;
	_fm[5] = _scale[5] * ft0.mz;
	if (_two_sensors){
		ft1 = read_ftdata(_filter_address, 1, 1);
		_fm[6] = _scale[0] * ft1.fx;
		_fm[7] = _scale[1] * ft1.fy;
		_fm[8] = _scale[2] * ft1.fz;
		_fm[9] = _scale[3] * ft1.mx;
		_fm[10] = _scale[4] * ft1.my;
		_fm[11] = _scale[5] * ft1.mz;
		num_doubles_ss = 6 * 2;
	}

	_transmission.send(_fm, num_doubles_ss);
}

int _tmain(int argc, _TCHAR* argv[], char* envp[])
{
	printf("\n----------------------------------------------");
	printf("\n");
	printf("\nTelerobotic and Biorobotic Systems Lab");
	printf("\nJR3 F/T sensor UDP streaming server");
	printf("\n");
	printf("\n----------------------------------------------");
	printf("\n");

	Config config(
		"jr3_sensor_udp_streaming_server.cfg",
		envp);

	string tmp = config.pString("ip_local").c_str();
	char ip_local_scp[1024];
	strcpy_s(ip_local_scp, tmp.c_str());
	tmp = config.pString("ip_remote").c_str();
	char ip_remote_scp[1024];
	strcpy_s(ip_remote_scp, tmp.c_str());
	short port_remote_ss = (short)config.pInt("port_remote");
	short port_local_ss = (short)config.pInt("port_local");
	_two_sensors = config.pBool("two_sensors");
	_processor_number = config.pInt("sensor");
	short sample_time_ss = config.pInt("ts");

	_filter_address = FILTER4;
	short filter_num_ss = config.pInt("filter");
	switch (filter_num_ss) {
		case 0:
			_filter_address = FILTER0;
			break;
		case 1:
			_filter_address = FILTER1;
			break;
		case 2:
			_filter_address = FILTER2;
			break;
		case 3:
			_filter_address = FILTER3;
			break;
		case 4:
			_filter_address = FILTER4;
			break;
		case 5:
			_filter_address = FILTER5;
			break;
		case 6:
			_filter_address = FILTER6;
			break;
	}

	printf("\nLocal IP address: %s:%d", ip_local_scp, port_local_ss);
	printf("\nRemote IP address: %s:%d", ip_remote_scp, port_remote_ss);
	printf("\nUsing two sensors? %s", _two_sensors ? "true" : "false");
	if(!_two_sensors) printf("\nSensor number: %d", _processor_number);
	printf("\nFilter number: %d", filter_num_ss);

	//TODO add error handling for init
	_transmission.init_transmission(ip_local_scp, port_local_ss,
		ip_remote_scp, port_remote_ss);

	if (init_jr3(0x1762, 0x3112, 1, 2, 1, 1)){
		printf("\nSensor init failed.");
		return 1;
	}
	else{
		//printf("\nSensor reply= %d", init_jr3(0x1762, 0x3112, 1, 1, 1, 1));
		printf("\nSensor init successful.");
	}

	if (_two_sensors) _processor_number = 0;

	printf("\nWaiting for sensor %d errors to clear...", _processor_number);
	while (read_jr3(ERRORS, _processor_number, 1));
	printf("Done!");
	printf("\nWaiting for sensor %d offsets to settle...", _processor_number);
	//while ((clock() / CLOCKS_PER_SEC) < 1000) {}
	Sleep(2000);
	printf("Done!");
	printf("\nResetting offsets...");
	short error = reset_offsets(_processor_number, 1);
	if (error){
		printf("Resetting offsets failed.");
	}
	else{
		printf("Success!");
	}

	if(_two_sensors){
		printf("\nWaiting for sensor 1 errors to clear...");
		while (read_jr3(ERRORS, 1, 1));
		printf("Done!");
		printf("\nWaiting for sensor 1 offsets to settle...");
		//while ((clock() / CLOCKS_PER_SEC) < 1000) {}
		Sleep(2000);
		printf("Done!");
		printf("\nResetting offsets...");
		short error = reset_offsets(1, 1);
		if (error){
			printf("Resetting offsets failed.");
		}
		else{
			printf("Success!");
		}
	}

	struct force_array fullScale;
	fullScale.fx = 100;	//get_full_scales(0, 1); // function call to get full scales
	fullScale.fy = 100;
	fullScale.fz = 200;
	fullScale.mx = 5;
	fullScale.my = 5;
	fullScale.mz = 5;

	_scale[0] = 1. / 16384 * fullScale.fx; //--> 14 bit (16384 dec) ADC
	_scale[1] = 1. / 16384 * fullScale.fy;
	_scale[2] = 1. / 16384 * fullScale.fz;
	_scale[3] = 1. / 16384 * fullScale.mx;
	_scale[4] = 1. / 16384 * fullScale.my;
	_scale[5] = 1. / 16384 * fullScale.mz;

	//Set up Multimedia Time Services
	if (timeGetDevCaps(&tc, sizeof(TIMECAPS)) != TIMERR_NOERROR)
	{
		return -1;
	}
	wTimerRes = min(max(tc.wPeriodMin, TIME_RESOLUTION*sample_time_ss), tc.wPeriodMax);
	timeBeginPeriod(wTimerRes);

	//Start timer callback
	MMRESULT timer = timeSetEvent(TIME_PERIOD, 0, timercbk, 0, TIME_PERIODIC);

	printf("\nStreaming to %s:%d at %.0fHz... \nPress any key to quit.", 
		ip_remote_scp, port_remote_ss, 1/(TIME_RESOLUTION*(double)sample_time_ss/1000.));
	while (!_kbhit());

	//Stop timer callback
	timeKillEvent(timer);
	timeEndPeriod(wTimerRes);
}
