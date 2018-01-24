// jr3_sensor_read_udp_client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>
#include <conio.h>

#include "tbs_jr3_ft_udp_client.h"
#include "config.h"

int _tmain(int argc, _TCHAR* argv[], char* envp[])
{

	Config config("jr3_sensor_read_udp_client.cfg", envp);
	string ip_addr_st = config.pString("ip_remote");
	char ip_addr_scp[1024];
	strcpy_s(ip_addr_scp, ip_addr_st.c_str());
	short port_ss = (short)config.pInt("port_remote");

	init_connection("0.0.0.0", 0, ip_addr_scp, port_ss);

	if (init_jr3(0x1762, 0x3112, 1, 2, 1, 1)){
		printf("\nSensor init failed.");
		//return 1;
	}
	else{
		printf("\nSensor init successful.");
	}

	// Sensor 0
	printf("\nWaiting for sensor 0 errors to clear...");
	while (read_jr3(ERRORS, 0, 1));
	printf("Done!");
	printf("\nWaiting for sensor 0 offsets to settle...");
	Sleep(2000);
	printf("Done!");
	printf("\nResetting offsets...");
	short error = reset_offsets(0, 1);
	if (error){
		printf("Resetting offsets failed.");
	}
	else{
		printf("Success!");
	}

	// Sensor 1
	printf("\nWaiting for sensor 1 errors to clear...");
	while (read_jr3(ERRORS, 1, 1));
	printf("Done!");
	printf("\nWaiting for sensor 1 offsets to settle...");
	Sleep(2000);
	printf("Done!");
	printf("\nResetting offsets...");
	error = reset_offsets(1, 1);
	if (error){
		printf("Resetting offsets failed.");
	}
	else{
		printf("Success!");
	}

	printf("\n");
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
	printf("\n");
	printf("Fx      Fy      Fz      Tx      Ty      Tz      ");
	printf("\n");
	do{
		
		force_array ft_p[2];
		read_ftdata_2(FILTER2, ft_p, 1);

		fm[0] = scale[0] * ft_p[0].fx;
		fm[1] = scale[1] * ft_p[0].fy;
		fm[2] = scale[2] * ft_p[0].fz;
		fm[3] = scale[3] * ft_p[0].mx;
		fm[4] = scale[4] * ft_p[0].my;
		fm[5] = scale[5] * ft_p[0].mz;
		fm[6] = scale[0] * ft_p[1].fx;
		fm[7] = scale[1] * ft_p[1].fy;
		fm[8] = scale[2] * ft_p[1].fz;
		fm[9] = scale[3] * ft_p[1].mx;
		fm[10] = scale[4] * ft_p[1].my;
		fm[11] = scale[5] * ft_p[1].mz;
		
		//printf("\r% 03.2f   % 03.2f   % 03.2f   % 03.2f   % 03.2f   % 03.2f   ",
		//	fm[0], fm[1], fm[2], fm[3], fm[4], fm[5]);
		printf("\r% 03.2f   % 03.2f   % 03.2f   % 03.2f   % 03.2f   % 03.2f   ",
			fm[6], fm[7], fm[8], fm[9], fm[10], fm[11]);
	} while (!_kbhit());

	printf("\n");
	system("Pause");
	return 0;
}
