

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#include <windows.h>
#include <crtdbg.h>
#include <stdio.h>
#include <iostream>
#include <conio.h>
#include <ctime>

using namespace std;

void init_transmission(char* ip_local_scp, short port_local_ss,
        char* ip_remote_scp, short port_remote_ss);
void init_transmission(char* ip_address_scp, short port);
short send(char* array_scp, int len);
short send(double* number_dbp, int len);
short send(float* array_flp, int len);
short send(int* array_sip, int len);
short send(short* number_ssp, int len);
bool listen(char* buffer_scp, int len);
