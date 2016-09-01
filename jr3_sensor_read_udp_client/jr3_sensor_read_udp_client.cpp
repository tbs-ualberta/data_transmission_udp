// jr3_sensor_read_udp_client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <iostream>
#include <conio.h>

#include "tbs_jr3_ft_udp_client.h"
#include "config.h"

int _tmain(int argc, _TCHAR* argv[], char* envp[])
{
	// Valid remote IP addresses:
	// 192.168.0.7: Needle insertion network
	// 142.244.62.48: ece.ualberta.ca network (lab subnet 142.244.62.*)

	Config config("jr3_sensor_read_udp_client.cfg", envp);
	string ip_addr_st = config.pString("ip_remote");
	char ip_addr_scp[1024];
	strcpy_s(ip_addr_scp, ip_addr_st.c_str());
	short port_ss = (short)config.pInt("port_remote");

	init_connection(ip_addr_scp, port_ss);

	if (init_jr3(0x1762, 0x3112, 1, 1, 1, 1)){
		printf("\nSensor init failed.");
		//return 1;
	}
	else{
		//printf("\nSensor reply= %d", init_jr3(0x1762, 0x3112, 1, 1, 1, 1));
		printf("\nSensor init successful.");
	}

	printf("\nWaiting for sensor 1 errors to clear...");
	//short dbg = read_jr3(ERRORS, 0, 1);
	while (read_jr3(ERRORS, 0, 1));
	printf("Done!");
	printf("\nWaiting for sensor 1 offsets to settle...");
	//while ((clock() / CLOCKS_PER_SEC) < 1000) {}
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

	printf("\n");
	/*
	struct force_array ft;
	do{
		ft = read_ftdata(FILTER2, 0, 1);
		printf("\n%d", ft.fx);
	} while (!_kbhit());
	*/
	struct force_array fullScale;
	fullScale.fx = 100;	//get_full_scales(0, 1); // function call to get full scales
	fullScale.fy = 100;
	fullScale.fz = 200;
	fullScale.mx = 5;
	fullScale.my = 5;
	fullScale.mz = 5;

	double scale[6];
	double fm[6];
	scale[0] = 1. / 16384 * fullScale.fx; //--> 14 bit (16384 dec) ADC
	scale[1] = 1. / 16384 * fullScale.fy;
	scale[2] = 1. / 16384 * fullScale.fz;
	scale[3] = 1. / 16384 * fullScale.mx;
	scale[4] = 1. / 16384 * fullScale.my;
	scale[5] = 1. / 16384 * fullScale.mz;

	// --- Read f/t data
	struct force_array ft;
	printf("\n");
	printf("Fx      Fy      Fz      Tx      Ty      Tz      ");
	printf("\n");
	do{
		ft = read_ftdata(FILTER2, 0, 1);

		fm[0] = scale[0] * ft.fx;
		fm[1] = scale[1] * ft.fy;
		fm[2] = scale[2] * ft.fz;
		fm[3] = scale[3] * ft.mx;
		fm[4] = scale[4] * ft.my;
		fm[5] = scale[5] * ft.mz;

		printf("\r% 03.2f   % 03.2f   % 03.2f   % 03.2f   % 03.2f   % 03.2f   ", fm[0], fm[1], fm[2], fm[3], fm[4], fm[5]);
	} while (!_kbhit());

	printf("\n");
	system("Pause");
	return 0;
}

