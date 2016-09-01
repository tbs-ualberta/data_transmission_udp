
#ifndef TBS_JR3_FT_UDP_CLIENT
#define TBS_JR3_FT_UDP_CLIENT

#include "tbs_jr3_ft_udp_client.h"

using namespace std;

SOCKET  g_socketC;
struct sockaddr_in g_serverInfo;
int g_len;

const unsigned char TAG_INIT		= 0;
const unsigned char TAG_ACK_INIT	= 1;
const unsigned char TAG_DATA		= 2;
const unsigned char TAG_FT_DATA		= 3;
const unsigned char TAG_REQ_FT_DATA	= 4;
const unsigned char TAG_REQ_DATA	= 5;
const unsigned char TAG_REQ_OS_RST	= 6;
const unsigned char TAG_ACK_OS_RST	= 7;

void init_connection(char* ip_addr, short port){

	// Init winsock
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		printf("\nWSAStartup failed with error: %d\n", iResult);
	}

	g_len = sizeof(g_serverInfo);
	g_serverInfo.sin_family = AF_INET;
	g_serverInfo.sin_port = htons(port);
	g_serverInfo.sin_addr.s_addr = inet_addr(ip_addr);


	g_socketC = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (g_socketC == INVALID_SOCKET) {
		printf("\nSocket failed with error: %ld\n", WSAGetLastError());
		WSACleanup();
	}
	g_socketC = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

}


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

unsigned char* short2chararray(unsigned short in_sh){
	unsigned char temp[2];
	temp[0] = (in_sh >> 8) & 0xFF;
	temp[1] = in_sh & 0x00FF;
	return temp;
}

// API functions
// Author: Thomas Lehmann, lehmann@ualberta.ca
//----------------------------------------------------------------------

// Read data from JR3
// Input parameter: address, processor number
// Return Value: Value stored at address
short read_jr3(unsigned short address, short processor_number, short na){

	char buffer_ch[1024];
	unsigned char tag;
	short buffer_sh[512];
	unsigned char* temp;

	//Step 1: Request data
	buffer_ch[0] = TAG_REQ_DATA;
	temp = short2chararray(address);
	buffer_ch[1] = temp[0];
	buffer_ch[2] = temp[1];
	buffer_ch[3] = processor_number;
	if (sendto(
		g_socketC, buffer_ch, sizeof(buffer_ch), 0, (sockaddr*)&g_serverInfo, g_len) == SOCKET_ERROR)
	{
		printf("\nsendto failed in 'read_jr3' with error: %d", WSAGetLastError());
	}

	//Step 2: Wait for data to receive
	recvfrom(
		g_socketC, buffer_ch, sizeof(buffer_ch), 0, (sockaddr*)&g_serverInfo, &g_len);
	tag = buffer_ch[0];
	short result;
	if (tag == TAG_DATA){
		result = chararray2short((unsigned char*)buffer_ch+1);
	}

	return result;
}

// Write data to JR3
// Input parameters: address, value_to_write, processor number
void write_jr3(unsigned short address, unsigned short value, short processor_number, short na){
  //processor_number --> channel
}

// Reset offsets with values of FILTER2
// Input parameters: processor number
// Return Value: 0 if command was successful
short reset_offsets(short processor_number, short na){

	char buffer_ch[1024];
	unsigned char tag;
	short buffer_sh[512];
	unsigned char* temp;

	//Step 1: Request data
	buffer_ch[0] = TAG_REQ_OS_RST;
	buffer_ch[1] = processor_number;
	if (sendto(
		g_socketC, buffer_ch, sizeof(buffer_ch), 0, (sockaddr*)&g_serverInfo, g_len) == SOCKET_ERROR)
	{
		printf("\nsendto failed in 'reset_offsets' with error: %d", WSAGetLastError());
	}

	//Step 2: Wait for data to receive
	recvfrom(
		g_socketC, buffer_ch, sizeof(buffer_ch), 0, (sockaddr*)&g_serverInfo, &g_len);
	tag = buffer_ch[0];
	short result;
	if (tag == TAG_ACK_OS_RST){
		result = chararray2short((unsigned char*)buffer_ch + 1);
	}

	return result;
}

// Read force/torque data
// Input parameters: filter address, processor number
// Return Value: F/T data in a force_array format
struct force_array read_ftdata(short filter_address, short processor_number, short na){
    //TODO Check if init was done

    char buffer_ch[1024];
    unsigned char tag;
    short buffer_sh[512];
    int i;
	unsigned char* temp_ch;
    struct force_array fm;

    // First step: Send data request to sensor
    buffer_ch[0] = TAG_REQ_FT_DATA;
    temp_ch = short2chararray(filter_address);
    buffer_ch[1] = temp_ch[0];
    buffer_ch[2] = temp_ch[1];
	buffer_ch[3] = (unsigned char)processor_number;
    if(sendto(
		g_socketC, buffer_ch, sizeof(buffer_ch), 0, (sockaddr*)&g_serverInfo, g_len) == SOCKET_ERROR)
	{
		printf("\nsendto failed in 'read_ftdata' with error: %d", WSAGetLastError());
	}
    // Second step: Wait for data & receive
    // Below should be a blocking call
	
    recvfrom(
		g_socketC, buffer_ch, sizeof(buffer_ch), 0, (sockaddr*)&g_serverInfo, &g_len);
    tag = buffer_ch[0];
    if(tag == TAG_FT_DATA){
        // Convert from char array (8 bit) to short array (16 bit)
        for(i=0; i<8; i++){
          buffer_sh[i] = chararray2short((unsigned char*)buffer_ch+i*2+1);
        }
    }
    else{
        //TODO make returned struct NULL
    }

	fm.fx = buffer_sh[0];
	fm.fy = buffer_sh[1];
	fm.fz = buffer_sh[2];
	fm.mx = buffer_sh[3];
	fm.my = buffer_sh[4];
	fm.mz = buffer_sh[5];
	fm.v1 = buffer_sh[6];
	fm.v2 = buffer_sh[7];

	return fm;
}


// Input Values: vendor_ID, device_ID, number_of_board, number_of_processors, download
// where
// number_of_board = 1 to single board system
// number_of_processors is the number of processors in the board (1 for simple PCI boards)
// download is a value that should be 1 (if code is to be downloaded) or any other value
// if code was already download and user wants only to open an handle to the board.

// 0 if success
// 1 if failed to create handle

short init_jr3(
    unsigned long vendor_ID, unsigned long device_ID,
    unsigned long number_of_board, short number_of_processors,
    short download, short na){
  // vendor_ID, device_ID, number_of_board, download is set on server
  char buffer_ch[1024];
  short error = 0;

  buffer_ch[0] = TAG_INIT;
  buffer_ch[1] = (char)number_of_processors;
  printf("\nSending init request...");
  if (sendto(
	  g_socketC, buffer_ch, sizeof(buffer_ch), 0, (sockaddr*)&g_serverInfo, g_len) == SOCKET_ERROR)
  {  
	  printf("\nsendto failed in 'init_jr3' with error: %d", WSAGetLastError());
	  return -2;
  }
  printf("Done!");
  // Wait for acknowledge messsage
  printf("\nWaiting for response...");
  recvfrom(
	  g_socketC, buffer_ch, sizeof(buffer_ch), 0, (sockaddr*)&g_serverInfo, &g_len);

  if(buffer_ch[0] == TAG_ACK_INIT){
    error = (short)buffer_ch[1];
	printf("Success!");
  }
  else{
    error = -1;
	printf("Fail!");
  }

  return error;
}

// Removes the environment
void close_jr3(short na){
	//TODO No idea what's supposed to happen here.
	//Just a dummy function so far such that code using the API won't crash.
}

#endif
