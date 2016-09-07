
#include "tbs_jr3_ft_udp_client.h"
#include "jr3_message_tags.h"

using namespace std;

SOCKET  g_socketC;
struct sockaddr_in g_serverInfo;
int g_len;
data_transmission transmission;

void init_connection(char* ip_local_scp, short port_local_ss,
        char* ip_remote_scp, short port_remote_ss){

	int error = transmission.init_transmission(
		ip_local_scp, port_local_ss, ip_remote_scp, port_remote_ss);
	if(error) printf("\nSocket init failed with error: %ld\n", error);
	else printf("\nSocket initialized on %s:%d.", ip_remote_scp, port_remote_ss);
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
	int comm_error = 0;

	//Step 1: Request data
	buffer_ch[0] = TAG_REQ_DATA;
	temp = transmission.short2chararray(address);
	buffer_ch[1] = temp[0];
	buffer_ch[2] = temp[1];
	buffer_ch[3] = processor_number;

	comm_error = transmission.send(buffer_ch, sizeof(buffer_ch));
	if (comm_error){
		printf("\nSending failed in 'read_jr3' with error: %d", comm_error);
	}

	//Step 2: Wait for data to be received
	comm_error = transmission.listen(buffer_ch, sizeof(buffer_ch));
	if (comm_error){
		printf("\nListening failed in 'read_jr3' with error: %d", comm_error);
	}
	tag = buffer_ch[0];
	short result;
	if (tag == TAG_DATA){
		result = transmission.chararray2short((unsigned char*)buffer_ch+1);
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
	int comm_error = 0;

	//Step 1: Request data
	buffer_ch[0] = TAG_REQ_OS_RST;
	buffer_ch[1] = processor_number;
	comm_error = transmission.send(buffer_ch, sizeof(buffer_ch));
	if (comm_error){
		printf("\nSending failed in 'reset_offsets' with error: %d",
			comm_error);
	}

	//Step 2: Wait for data to receive
	comm_error = transmission.listen(buffer_ch, sizeof(buffer_ch));
	if(comm_error){
		printf("Listening failed in 'reset_offsets' with error: %d",
			comm_error);
	}
	tag = buffer_ch[0];
	short result;
	if (tag == TAG_ACK_OS_RST){
		result = transmission.chararray2short((unsigned char*)buffer_ch + 1);
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
	unsigned char* temp_ch;
    struct force_array fm;
	int comm_error = 0;

    // First step: Send data request to sensor
    buffer_ch[0] = TAG_REQ_FT_DATA;
    temp_ch = transmission.short2chararray(filter_address);
    buffer_ch[1] = temp_ch[0];
    buffer_ch[2] = temp_ch[1];
	buffer_ch[3] = (unsigned char)processor_number;
	comm_error = transmission.send(buffer_ch, sizeof(buffer_ch));
    if(comm_error){
		printf("\nSending failed in 'read_ftdata' with error: %d", comm_error);
	}
    // Second step: Wait for data & receive
    // Below should be a blocking call

	comm_error = transmission.listen(buffer_ch, sizeof(buffer_ch));
    tag = buffer_ch[0];
    if(tag == TAG_FT_DATA){
        // Convert from char array (8 bit) to short array (16 bit)
        for(int i=0; i<8; i++){
        	buffer_sh[i] = transmission.chararray2short(
                (unsigned char*)buffer_ch+i*2+1);
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
	int comm_error = 0;
	short init_error = 0;

	buffer_ch[0] = TAG_INIT;
	buffer_ch[1] = (char)number_of_processors;

	printf("\nSending init request...");
	comm_error = transmission.send(buffer_ch, sizeof(buffer_ch));
	if (comm_error){
		printf("\nSending failed in 'init_jr3' with error: %d", comm_error);
		return -2;
	}
	printf("Done!");
	// Wait for acknowledge messsage
	printf("\nWaiting for response...");
	comm_error = transmission.listen(buffer_ch, sizeof(buffer_ch));
	if(comm_error){
		printf("\nReceiving failed in 'init_jr3' with error: %d", comm_error);
		return -2;
	}

	if(buffer_ch[0] == TAG_ACK_INIT){
		init_error = (short)buffer_ch[1];
		printf("Success!");
	}
	else{
		init_error = -1;
		printf("Fail!");
	}

	return init_error;
}

// Removes the environment
void close_jr3(short na){
	//TODO No idea what's supposed to happen here.
	//Just a dummy function so far such that code using the API won't crash.
}
