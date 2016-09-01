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

const unsigned char TAG_INIT		= 0;
const unsigned char TAG_ACK_INIT	= 1;
const unsigned char TAG_DATA		= 2;
const unsigned char TAG_FT_DATA		= 3;
const unsigned char TAG_REQ_FT_DATA = 4;
const unsigned char TAG_REQ_DATA	= 5;
const unsigned char TAG_REQ_OS_RST	= 6;
const unsigned char TAG_ACK_OS_RST	= 7;
const unsigned char TAG_RNDTRIP		= 8;


short chararray2short(unsigned char *in_ch){
	short temp = in_ch[0];
	temp = temp << 8;
	return temp | in_ch[1];
}

unsigned char* short2chararray(short in_sh){
	unsigned char temp[2];
	temp[0] = (in_sh >> 8) & 0xFF;
	temp[1] = in_sh & 0x00FF;
	return temp;
}


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

	init_transmission(ip_local_scp, port_local_ss);

	unsigned char* tmp;
	short filter_address;

	printf("\nListening on %s:%d...", ip_local_scp, port_local_ss);
	while (1){
		char buffer_rec[1024];
		char buffer_snd[1024];
		ZeroMemory(buffer_rec, sizeof(buffer_rec));
		ZeroMemory(buffer_snd, sizeof(buffer_snd));
		if (!listen(buffer_rec, sizeof(buffer_rec)))
		{
			switch (buffer_rec[0]) {
			case TAG_INIT:
				//printf("\nInitializing...");
				buffer_snd[0] = TAG_ACK_INIT;
				if (!init_jr3(0x1762, 0x3112, 1, (short)buffer_rec[1], 1, 1)){
					buffer_snd[1] = 0;
					//printf("Success!");
				}
				else{
					buffer_snd[1] = 1;
					printf("Failed.");
				}
				break;
			case TAG_REQ_FT_DATA:
				filter_address = chararray2short((unsigned char*)buffer_rec + 1);
				buffer_snd[0] = TAG_FT_DATA;
				for (int i = 0; i<8; i++){
					tmp = short2chararray(
						read_jr3(filter_address + i, buffer_rec[3], 1));
					buffer_snd[i * 2 + 1] = tmp[0];
					buffer_snd[i * 2 + 2] = tmp[1];
				}
				break;
			case TAG_REQ_DATA:
				filter_address = chararray2short((unsigned char*)buffer_rec + 1);
				tmp = short2chararray(read_jr3(filter_address, buffer_rec[3], 1));
				buffer_snd[0] = TAG_DATA;
				buffer_snd[1] = tmp[0];
				buffer_snd[2] = tmp[1];
				break;
			case TAG_REQ_OS_RST:
				tmp = short2chararray(reset_offsets(buffer_rec[1], 1));
				buffer_snd[0] = TAG_ACK_OS_RST;
				buffer_snd[1] = tmp[0];
				buffer_snd[2] = tmp[1];
				break;
			case TAG_RNDTRIP:
				buffer_snd[0] = buffer_rec[0];
				break;
			}
			send(buffer_snd, sizeof(buffer_snd));
		}
	}
}
