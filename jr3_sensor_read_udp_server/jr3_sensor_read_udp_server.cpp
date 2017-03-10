// jr3_sensor_read_udp_server.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <crtdbg.h>
#include <stdio.h>
#include <iostream>
#include <conio.h>
#include <ctime>

#pragma pack(1)
#include "jr3pci_ft.h"
#include "config.h"
#include "data_transmission.h"
#include "jr3_message_tags.h"

int _tmain(int argc, _TCHAR* argv[], char* envp[]) {

	printf("\n----------------------------------------------");
	printf("\n");
	printf("\nTelerobotic and Biorobotic Systems Lab");
	printf("\nJR3 F/T sensor UDP server");
	printf("\n");
	printf("\n----------------------------------------------");
	printf("\n");

	Config config(
		"jr3_sensor_read_udp_server.cfg",
		envp);

	string tmp_st = config.pString("ip_local").c_str();
	char ip_local_scp[1024];
	strcpy_s(ip_local_scp, tmp_st.c_str());
	short port_local_ss = (short)config.pInt("port_local");
	int comm_error = 0;

	data_transmission transmission;

	comm_error = transmission.init_transmission(ip_local_scp, port_local_ss);
	if(comm_error) printf("\nSocket init failed with error: %ld\n", comm_error);
	else printf("\nSocket initialized on %s:%d.", ip_remote_scp, port_remote_ss);

	unsigned char* tmp;
	short filter_address;

	printf("\nListening on %s:%d...", ip_local_scp, port_local_ss);
	while (!comm_error){
		//TODO reduce the buffer size
		char buffer_rec[1024];
		char buffer_snd[1024];
		ZeroMemory(buffer_rec, sizeof(buffer_rec));
		ZeroMemory(buffer_snd, sizeof(buffer_snd));
		comm_error = transmission.listen(buffer_rec, sizeof(buffer_rec));
		short amnt_sensors_ss = 0;
		if(!comm_error)
		{
			switch (buffer_rec[0]) {
			case TAG_INIT:
				buffer_snd[0] = TAG_ACK_INIT;
				amnt_sensors_ss = (short)buffer_rec[1];
				if (!init_jr3(0x1762, 0x3112, 1, amnt_sensors_ss, 1, 1)){
					buffer_snd[1] = 0;
				}
				else{
					buffer_snd[1] = 1;
				}
				break;
			case TAG_REQ_FT_DATA:
				filter_address = transmission.chararray2short(
					(unsigned char*)buffer_rec + 1);
				buffer_snd[0] = TAG_FT_DATA;
				for (int i = 0; i<8; i++){
					tmp = transmission.short2chararray(
						read_jr3(filter_address + i, buffer_rec[3], 1));
					buffer_snd[i * 2 + 1] = tmp[0];
					buffer_snd[i * 2 + 2] = tmp[1];
				}
				break;
			case TAG_REQ_FT_DATA_2:
				filter_address = transmission.chararray2short(
					(unsigned char*)buffer_rec + 1);
				buffer_snd[0] = TAG_FT_DATA_2;
				for (int i = 0; i<8; i++){
					tmp = transmission.short2chararray(
						read_jr3(filter_address + i, 0, 1));
					buffer_snd[i*2+1] = tmp[0];
					buffer_snd[i*2+2] = tmp[1];
					tmp = transmission.short2chararray(
						read_jr3(filter_address + i, 1, 1));
					buffer_snd[i*2+1+8] = tmp[0];
					buffer_snd[i*2+2+8] = tmp[1];
				}
				break;
			case TAG_REQ_DATA:
				filter_address = transmission.chararray2short(
					(unsigned char*)buffer_rec + 1);
				tmp = transmission.short2chararray(
					read_jr3(filter_address, buffer_rec[3], 1));
				buffer_snd[0] = TAG_DATA;
				buffer_snd[1] = tmp[0];
				buffer_snd[2] = tmp[1];
				break;
			case TAG_REQ_OS_RST:
				tmp = transmission.short2chararray(
					reset_offsets(buffer_rec[1], 1));
				buffer_snd[0] = TAG_ACK_OS_RST;
				buffer_snd[1] = tmp[0];
				buffer_snd[2] = tmp[1];
				break;
			case TAG_RNDTRIP:
				buffer_snd[0] = buffer_rec[0];
				break;
			}

			comm_error = transmission.send(buffer_snd, sizeof(buffer_snd));
			if(comm_error)
				printf("\nSending failed with error: %d", comm_error);
		}
		else{
			printf("\nListening failed with error: %d", comm_error);
		}
	}
}
