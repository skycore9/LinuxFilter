/*
 * Name:	Packet.h
 *
 * Author:	Feng Wei
 *
 * Date:	2015-02-15
 */

#ifndef PACKET_H
#define PACKET_H

#include "Session.h"

typedef struct Packet
{
	char			inEthNum;
	FiveGroup 		fiveGroup;
	short 			packetSize;	/* units are B */
	void 			*p_l4hdr;
	unsigned char 	*p_Payload;
	SessionHashNode *p_Session;
	int 			userLevel;
	int				permitted2pass;
}Packet;

#endif
