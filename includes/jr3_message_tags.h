#ifndef JR3_MESSAGE_TAGS_H
#define JR3_MESSAGE_TAGS_H

/*
 * The below tags are sent as the first byte in a data package. The purpose is
 * to tag the sent/received data bytes comprising a message.
 * A message looks as follows: [tag byte][data byte][data byte]...
 * The tagging serves two purposes:
 * - As a command for the receiver to determine which action to perform
 * - Determining how data contained in the bytes following the tag byte
 *   should be converted and interpreted
 */

const unsigned char TAG_INIT		    =  0;  // Initialize the JR3 DAQ card
const unsigned char TAG_ACK_INIT	    =  1;  // Acknowledge that the initialization was carried out
const unsigned char TAG_DATA		    =  2;  // The package contains data read from the DAQ card's address registers (returned from read_jr3())
const unsigned char TAG_FT_DATA		    =  3;  // The package contains raw force/torque data (returned from read_ftdata())
const unsigned char TAG_REQ_FT_DATA     =  4;  // Request force/torque data (returned from read_ftdata())
const unsigned char TAG_REQ_DATA	    =  5;  // Request sensor data (returned from read_jr3())
const unsigned char TAG_REQ_OS_RST	    =  6;  // Request an offset reset
const unsigned char TAG_ACK_OS_RST      =  7;  // Acknowledge that the offset reset was carried out
const unsigned char TAG_RNDTRIP		    =  8;  // Roundtrip package: Immediately return the same package for measureing the round trip delay
const unsigned char TAG_REQ_FT_DATA_2   =  9;  // Request force/torque data from two sensors (returned from read_ftdata())
const unsigned char TAG_FT_DATA_2		= 10;  // The package contains raw force/torque data from two sensors (returned from read_ftdata())

#endif
