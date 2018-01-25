// udp_streaming_client.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "data_transmission.h"
#include "config.h"


int _tmain(int argc, _TCHAR* argv[], char* envp[])
{
	data_transmission transmission;

	Config config("udp_streaming_client.cfg", envp);
	string ip_addr_st = config.pString("ip_local");
	char ip_local_scp[1024];
	strcpy_s(ip_local_scp, ip_addr_st.c_str());
	short port_local_ss = (short)config.pInt("port_local");

	int error = transmission.init_transmission(ip_local_scp, port_local_ss);
	if (error) printf("\nSocket init failed with error: %ld\n", error);
	else printf("\nSocket initialized on %s:%d.", ip_local_scp, port_local_ss);

	char buffer_ch[1024];
	double* data;
	int comm_error = 0;
	printf("\nPress any key to quit...");
	printf("\n\n");
	do{
		comm_error = transmission.listen(buffer_ch, sizeof(buffer_ch));
		if (comm_error){
			printf("Listening failed with error: %d", comm_error);
		}
		else{
			data = (double*)buffer_ch;
			printf("%03.2f, %03.2f, %03.2f\r", data[0], data[1], data[2]);
		}
	} while (!_kbhit() && !comm_error);

	printf("\n");
	system("Pause");
	return 0;
}

