#ifndef TBS_JR3_FT_UDP_CLIENT
#define TBS_JR3_FT_UDP_CLIENT

#include <crtdbg.h>
#include <string>
#include "jr3pci_ft_tbs.h"
#include "data_transmission.h"

void init_connection(char*, short, char*, short);

// Read force/torque data from two sensors
// Input parameters: filter address, pointer to array of force_array struct
// Return Value: Communication error returned by WSAGetLastError(). If no error,
// return value is 0.
int read_ftdata_2(short filter_address, struct force_array* fm_ptr, short na);

#endif
