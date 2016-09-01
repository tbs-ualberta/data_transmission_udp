#include "data_transmission.h"

SOCKET socketS;
struct sockaddr_in local;
struct sockaddr_in remote;
int remotelen;

void InitWinsock()
{
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
}

void init_transmission(char* ip_local_scp, short port_local_ss){

    InitWinsock();
    remotelen = sizeof(remote);
    local.sin_family = AF_INET;
    local.sin_port = htons(port_local_ss);
    local.sin_addr.s_addr = inet_addr(ip_local_scp);

    socketS = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	bind(socketS, (sockaddr*)&local, sizeof(local));
}

void init_transmission(char* ip_local_scp, short port_local_ss,
        char* ip_remote_scp, short port_remote_ss){

    InitWinsock();
	remotelen = sizeof(remote);
	local.sin_family = AF_INET;
	local.sin_port = htons(port_local_ss);
	local.sin_addr.s_addr = inet_addr(ip_local_scp);
	remote.sin_family = AF_INET;
	remote.sin_port = htons(port_remote_ss);
	remote.sin_addr.s_addr = inet_addr(ip_remote_scp);

	socketS = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
}

short do_send(char* buffer_scp, int len){
	short error = sendto(
		socketS, buffer_scp, len, 0, (sockaddr*)&remote, remotelen);
	return error;
}

short send(char* array_scp, int len){
	return do_send((char*)array_scp, len);
}

short send(double* array_dbp, int len){
    return do_send((char*)array_dbp, len*8);
}

short send(float* array_flp, int len){
    return do_send((char*)array_flp, len*4);
}

short send(int* array_sip, int len){
    return do_send((char*)array_sip, len*4);
}

short send(short* array_ssp, int len){
    return do_send((char*)array_ssp, len*2);
}

bool listen(char* buffer_scp, int len){
    bool socket_error = recvfrom(
        socketS, buffer_scp, len, 0, (sockaddr*)&remote, &remotelen)
		== SOCKET_ERROR;
    return socket_error;
}
