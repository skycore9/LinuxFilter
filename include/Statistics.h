/*
 * Name:	Statistics.h
 *
 * Author:	Feng Wei
 *
 * Date:	2015-02-15
 */

#ifndef STATISTICS_H
#define STATISTICS_H

#define MAX_INFO_LOG		16
#define MAX_INFO_NUM		(1<<MAX_INFO_LOG)
#define WRITE_DB_INTVAL		10

typedef struct SessionInfo
{
	unsigned int			inIP;
	unsigned int			exIP;
	unsigned short			inPort;
	unsigned short			exPort;
	unsigned char			protocol;
	unsigned long long		begin;
	unsigned long long		end;
	int						appID;
	unsigned long long		pakcets;
	unsigned long long		bytes;
}SessionInfo;

typedef struct SessionData
{
	SessionInfo		sessionInfo[MAX_INFO_NUM];
	int				sessionNum;
}SessionData;

#endif
