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

using namespace std;

#pragma pack(1)


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
	bool two_sensors = config.pBool("two_sensors");
	short processor_number = config.pInt("sensor");

	short filter_address = FILTER4;
	short filter_num_ss = config.pInt("filter");
	switch (filter_num_ss) {
		case 0:
			filter_address = FILTER0;
			break;
		case 1:
			filter_address = FILTER1;
			break;
		case 2:
			filter_address = FILTER2;
			break;
		case 3:
			filter_address = FILTER3;
			break;
		case 4:
			filter_address = FILTER4;
			break;
		case 5:
			filter_address = FILTER5;
			break;
		case 6:
			filter_address = FILTER6;
			break;
	}

	printf("\nLocal IP address: %s:%d", ip_local_scp, port_local_ss);
	printf("\nRemote IP address: %s:%d", ip_remote_scp, port_remote_ss);
	printf("\nUsing two sensors? %s", two_sensors ? "true" : "false");
	if(!two_sensors) printf("\nSensor number: %d", processor_number);
	printf("\nFilter number: %d", filter_num_ss);

	init_transmission(ip_local_scp, port_local_ss,
		ip_remote_scp, port_remote_ss);

	if (init_jr3(0x1762, 0x3112, 1, 2, 1, 1)){
		printf("\nSensor init failed.");
		return 1;
	}
	else{
		//printf("\nSensor reply= %d", init_jr3(0x1762, 0x3112, 1, 1, 1, 1));
		printf("\nSensor init successful.");
	}

	if (two_sensors) processor_number = 0;

	printf("\nWaiting for sensor %d errors to clear...", processor_number);
	while (read_jr3(ERRORS, processor_number, 1));
	printf("Done!");
	printf("\nWaiting for sensor %d offsets to settle...", processor_number);
	//while ((clock() / CLOCKS_PER_SEC) < 1000) {}
	Sleep(2000);
	printf("Done!");
	printf("\nResetting offsets...");
	short error = reset_offsets(processor_number, 1);
	if (error){
		printf("Resetting offsets failed.");
	}
	else{
		printf("Success!");
	}

	if(two_sensors){
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

	double scale[6];
	double fm[12];
	scale[0] = 1. / 16384 * fullScale.fx; //--> 14 bit (16384 dec) ADC
	scale[1] = 1. / 16384 * fullScale.fy;
	scale[2] = 1. / 16384 * fullScale.fz;
	scale[3] = 1. / 16384 * fullScale.mx;
	scale[4] = 1. / 16384 * fullScale.my;
	scale[5] = 1. / 16384 * fullScale.mz;

	// --- Read f/t data
	struct force_array ft0;
	struct force_array ft1;

	printf("\nStreaming to %s:%d... \nPress any key to quit.", ip_remote_scp, port_remote_ss);
	short num_doubles_ss = 6;

	while (!_kbhit()){
		ft0 = read_ftdata(filter_address, processor_number, 1);
		fm[0] = scale[0] * ft0.fx;
		fm[1] = scale[1] * ft0.fy;
		fm[2] = scale[2] * ft0.fz;
		fm[3] = scale[3] * ft0.mx;
		fm[4] = scale[4] * ft0.my;
		fm[5] = scale[5] * ft0.mz;
		if(two_sensors){
			ft1 = read_ftdata(filter_address, 1, 1);
			fm[6] = scale[0] * ft1.fx;
			fm[7] = scale[1] * ft1.fy;
			fm[8] = scale[2] * ft1.fz;
			fm[9] = scale[3] * ft1.mx;
			fm[10] = scale[4] * ft1.my;
			fm[11] = scale[5] * ft1.mz;
			num_doubles_ss = 6*2;
		}

		send(fm, num_doubles_ss);
	}
}
