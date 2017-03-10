
#define _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#include <crtdbg.h>
#include <stdio.h>
#include "JR3PCIIoctls.h"
#pragma pack(1)
#include "jr3pci_ft_tbs.h"


HANDLE g_hJr3PciDevice;

// API functions
// Author: Thomas Lehmann, lehmann@ualberta.ca
//----------------------------------------------------------------------

// Read data from JR3
// Input parameter: address, processor number
// Return Value: Value stored at address
short read_jr3(unsigned short address, short processor_number, short na){

	JR3PCI_READ_WORD_REQUEST_PARAMS ReadWordRequestParams;
	JR3PCI_READ_WORD_RESPONSE_PARAMS ReadWordResponseParams;
	ReadWordRequestParams.ucChannel = (UCHAR)processor_number;
	ReadWordRequestParams.ulOffset = (ULONG)address;

	//TODO implement some sort of check whether init has been done before writing

	DWORD dwBytesReturned = 0;
	BOOL bSuccess = DeviceIoControl(
		g_hJr3PciDevice,					// handle to device
		IOCTL_JR3PCI_READ_WORD,					// operation
		&ReadWordRequestParams,				// input data buffer
		sizeof(JR3PCI_READ_WORD_REQUEST_PARAMS),  // size of input data buffer
		&ReadWordResponseParams,				// output data buffer
		sizeof(JR3PCI_READ_WORD_RESPONSE_PARAMS), // size of output data buffer
		&dwBytesReturned,						// byte count
		NULL);									// overlapped information

	// For debug...
	//_ASSERTE(bSuccess && (dwBytesReturned == sizeof(JR3PCI_READ_WORD_RESPONSE_PARAMS)));
	//_ASSERTE(ReadWordResponseParams.iStatus == JR3PCI_STATUS_OK);

	return ReadWordResponseParams.usData;
}

// Write data to JR3
// Input parameters: address, value_to_write, processor number
void write_jr3(unsigned short address, unsigned short value, short processor_number, short na){

	//processor_number --> channel

	JR3PCI_WRITE_WORD_REQUEST_PARAMS WriteWordRequestParams;
	WriteWordRequestParams.ucChannel = (UCHAR) processor_number;
	WriteWordRequestParams.ulOffset = (ULONG) address;
	WriteWordRequestParams.usData = value;

	JR3PCI_WRITE_WORD_RESPONSE_PARAMS WriteWordResponseParams;

	//TODO implement some sort of check whether init has been done before writing

	DWORD dwBytesReturned = 0;
	BOOL bSuccess = DeviceIoControl(
		g_hJr3PciDevice,					// handle to device
		IOCTL_JR3PCI_WRITE_WORD,					// operation
		&WriteWordRequestParams,				// input data buffer
		sizeof(JR3PCI_WRITE_WORD_REQUEST_PARAMS),  // size of input data buffer
		&WriteWordResponseParams,				// output data buffer
		sizeof(JR3PCI_WRITE_WORD_RESPONSE_PARAMS), // size of output data buffer
		&dwBytesReturned,						// byte count
		NULL);									// overlapped information

	// For debugging...
	//_ASSERTE(bSuccess && (dwBytesReturned == sizeof(JR3PCI_WRITE_WORD_RESPONSE_PARAMS)));
	//_ASSERTE(WriteWordResponseParams.iStatus == JR3PCI_STATUS_OK);
}

// Reset offsets with values of FILTER2
// Input parameters: processor number
// Return Value: 0 if command was successful
short reset_offsets(short processor_number, short na){
	/*
	short filter_address = FILTER3;

	for (USHORT i = 0; i < 6; i++){
		short os = read_jr3(filter_address + i, processor_number, na);
		write_jr3(OFFSETS + i, os, processor_number, na);
		write_jr3(COMMAND_W0, 0x0700, processor_number, na);
	}
	*/
	write_jr3(COMMAND_W0, 0x0800, processor_number, na);

	return 0;
}

// Read force/torque data
// Input parameters: filter address, processor number
// Return Value: F/T data in a force_array format
struct force_array read_ftdata(short filter_address, short processor_number, short na){

	struct force_array fm;

	fm.fx = read_jr3(filter_address, processor_number, 1);
	fm.fy = read_jr3(filter_address + 1, processor_number, 1);
	fm.fz = read_jr3(filter_address + 2, processor_number, 1);
	fm.mx = read_jr3(filter_address + 3, processor_number, 1);
	fm.my = read_jr3(filter_address + 4, processor_number, 1);
	fm.mz = read_jr3(filter_address + 5, processor_number, 1);
	fm.v1 = read_jr3(filter_address + 6, processor_number, 1);
	fm.v2 = read_jr3(filter_address + 7, processor_number, 1);

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
short init_jr3(unsigned long vendor_ID, unsigned long device_ID, unsigned long number_of_board, short number_of_processors, short download, short na){

	char szDeviceName[30];
	sprintf(szDeviceName, "\\\\.\\JR3PCI%d", (int)number_of_board);

	////////////////////////////////////
	// Open a handle to the device.
	////////////////////////////////////
	g_hJr3PciDevice = CreateFileA(
		szDeviceName,					// file name
		GENERIC_READ | GENERIC_WRITE,   // access mode
		0,								// share mode
		NULL,							// SD
		OPEN_EXISTING,					// how to create
		0,								// file attributes
		NULL);							// handle to template file

	if(g_hJr3PciDevice == INVALID_HANDLE_VALUE)
	{
		return 1;
	}
	return 0;
}

// Removes the environment
void close_jr3(short na){
	//TODO No idea what's supposed to happen here.
	//Just a dummy function so far such that code using the API won't crash.
}

// Set translation of sensor frame about axis
// Input values: link type, transform num, translation in mm*10, processor number
// transform num: where in the table the transformation value should be stored (0, 1, 2...)
// translation: by how much should be translated (mm*10)
short set_translation(enum link_types link_types_en, short transform_num, short translation, short processor_number, short na){

	short type_ss = 0;
	switch (link_types_en)
	{
	case tx:
		type_ss = 1;
		break;
	case ty:
		type_ss = 2;
		break;
	case tz:
		type_ss = 3;
		break;
	default:
		return -1;
	}
	// The type of transformation (about Z-axis)
	transform_num = transform_num * 0x10;
	write_jr3(TRANSFORMS + transform_num, type_ss, processor_number, 1);
	// The amount of translation
	write_jr3(TRANSFORMS + transform_num + 1, translation, processor_number, 1);
	// That's it for transformation
	write_jr3(TRANSFORMS + transform_num + 2, 0, processor_number, 1);

	//Wait for completion
	while (read_jr3(ERRORS, processor_number, 1));

	return read_jr3(COMMAND_W0, processor_number, 1);
}

// Use Transform
//Input parameters: transform num, processor number
// Return Value: 0 if command was successful
short use_transform(short transform_num, short processor_number, short na){

	write_jr3(COMMAND_W0, 0x500 + transform_num, processor_number, 1);

	//Wait for completion
	while (read_jr3(ERRORS, processor_number, 1));
	return read_jr3(COMMAND_W0, processor_number, 1);
}
