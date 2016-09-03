#include "data_transmission.h"

SOCKET socketS;
struct sockaddr_in local;
struct sockaddr_in remote;
int remotelen;

void InitWinsock()
{
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != NO_ERROR) {
		printf("\nWSAStartup failed with error: %d\n", iResult);
	}
}

int init_transmission(char* ip_local_scp, short port_local_ss){

	int error = 0;
    InitWinsock();
    remotelen = sizeof(remote);
    local.sin_family = AF_INET;
    local.sin_port = htons(port_local_ss);
    local.sin_addr.s_addr = inet_addr(ip_local_scp);

    socketS = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	bind(socketS, (sockaddr*)&local, sizeof(local));
	if (socketS == INVALID_SOCKET) {
		error = WSAGetLastError();
		WSACleanup();
	}

	return error;
}

int init_transmission(char* ip_local_scp, short port_local_ss,
        char* ip_remote_scp, short port_remote_ss){

	int error = 0;
    InitWinsock();
	remotelen = sizeof(remote);
	local.sin_family = AF_INET;
	local.sin_port = htons(port_local_ss);
	local.sin_addr.s_addr = inet_addr(ip_local_scp);
	remote.sin_family = AF_INET;
	remote.sin_port = htons(port_remote_ss);
	remote.sin_addr.s_addr = inet_addr(ip_remote_scp);

	socketS = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (socketS == INVALID_SOCKET) {
		error = WSAGetLastError();
		WSACleanup();
	}

	return error;
}

int do_send(char* buffer_scp, int len){
	int error = 0;
	if(sendto(
			socketS, buffer_scp, len, 0, (sockaddr*)&remote, remotelen)
			== SOCKET_ERROR){
		error = WSAGetLastError();
	}
	return error;
}

int send(char* array_scp, int len){
	return do_send((char*)array_scp, len);
}

int send(double* array_dbp, int len){
    return do_send((char*)array_dbp, len*8);
}

int send(float* array_flp, int len){
    return do_send((char*)array_flp, len*4);
}

int send(int* array_sip, int len){
    return do_send((char*)array_sip, len*4);
}

int send(short* array_ssp, int len){
    return do_send((char*)array_ssp, len*2);
}

int send(unsigned short* array_usp, int len){
    return do_send((char*)array_usp, len*2);
}

int listen(char* buffer_scp, int len){
	int error = 0;
    if(recvfrom(
        	socketS, buffer_scp, len, 0, (sockaddr*)&remote, &remotelen)
			== SOCKET_ERROR){
		error = WSAGetLastError();
	}
    return error;
}

void num2charray(short in_ss, char* out_scp){
	out_scp = (char*)&in_ss;
}

void num2charray(short in_ss, char* out_scp, int* len_out_sip){
	out_scp = (char*)&in_ss;
	*len_out_sip = 2;
}

void num2charray(unsigned short in_us, char* out_scp){
	out_scp = (char*)&in_us;
}

void num2charray(unsigned short in_us, char* out_scp, int* len_out_sip){
	out_scp = (char*)&in_us;
	*len_out_sip = 2;
}

void charray2num(char* in_scp, short* out_ssp){
	out_ssp = (short*)in_scp;
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
