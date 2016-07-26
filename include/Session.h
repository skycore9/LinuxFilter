/*
 * Name:	Session.h
 *
 * Author:	Feng Wei
 *
 * Date:	2015-02-15
 */

#ifndef SESSION_H
#define SESSION_H

#define TCP_PROTO					6
#define UDP_PROTO					17

#define SESSION_TABLE_SIZE_LOG 		18
#define SESSION_TABLE_SIZE 			(1<<SESSION_TABLE_SIZE_LOG)
#define SESSION_TABLE_HASHCOVER		(SESSION_TABLE_SIZE-1)

#define SESSION_ON					0
#define SESSION_OFF					1

#define R_SESSION					1
#define F_SESSION					0

#define NEED_ANALYSIS				1
#define NO_MORE_ANALYSIS			0

#define DEFAULT_IN_ETH				'0'
#define DEFAULT_EX_ETH				'1'

#define BUCKET_SIZE					10000
#define UDP_SESSION_MAX_LIVE		5000 /* need reset */
#define TCP_SESSION_MAX_LIVE		3600000
#define MAX_ANALYSIS_PACKETS		40

typedef struct FiveGroup
{
	unsigned int	ip_src;
	unsigned int	ip_dst;
	unsigned short	th_sport;
	unsigned short	th_dport;
	unsigned char	ip_p;
}FiveGroup;

typedef struct TokenBucket
{
	int bucketSize;	/* unit is B */
	int tokens;		/* unit is B */
}TokenBucket;

typedef struct SessionHashNode
{
	struct SessionHashNode *pre;
	struct SessionHashNode *next;

	int appID;
	FiveGroup fiveGroup;
	long long packets;
	long long bytes;
	unsigned long long begin;
	unsigned long long lastPktTime;
	char analysis;
	char status;
	TokenBucket tokenBucket;
}SessionHashNode;

#endif
