/*
 * Name:	TTCN_BandwidthManage.c
 *
 * Author:	Feng Wei
 *
 * Date:	2015-02-16
 */

#include <linux/time.h>
#include "GlobalStruct.h"

static int IsInTimeRange(AppConfig *p_AppConfig, struct timeval *p_TimeVal)
{
	struct tm	currentTm;
	int			minutes;

	if (p_AppConfig->beginTime == UNDEFINED_TIME)
	{
		return IN_RANGE;
	}
	else
	{
		time_to_tm(p_TimeVal->tv_sec, 0, &currentTm);
		minutes = currentTm.tm_hour * 60 + currentTm.tm_min;

		if (minutes > p_AppConfig->beginTime && minutes < p_AppConfig->endTime)
			return IN_RANGE;
		else
			return OUT_RANGE;
	}
}

static int TokenBucketAlg(Packet *p_Packet, struct timeval *p_TimeVal,
		AppConfig *p_AppConfig)
{
	unsigned long long	mseconds;
	int 				interval;
	int					putinTokens;
	SessionHashNode		*p_Session = NULL;

	p_Session = p_Packet->p_Session;
	mseconds = 
		((unsigned long long)p_TimeVal->tv_sec)*1000+(p_TimeVal->tv_usec)/1000;

	if (IsInTimeRange(p_AppConfig, p_TimeVal) == IN_RANGE)
	{
		if (p_AppConfig->bandwidth == PKT_PASS)
		{
			return PKT_PASS;
		}
		else if (p_AppConfig->bandwidth == PKT_DROP)
		{
			return PKT_DROP;
		}

		interval = (int)(mseconds - p_Session->lastPktTime);
		putinTokens = interval * p_AppConfig->bandwidth;

		if (p_Session->tokenBucket.tokens + putinTokens > 
				p_Session->tokenBucket.bucketSize)
		{
			p_Session->tokenBucket.tokens = p_Session->tokenBucket.bucketSize;
		}
		else
		{
			p_Session->tokenBucket.tokens += putinTokens;
		}

		printk(KERN_INFO "interval: %d", interval);
		printk(KERN_INFO "putin tokens: %d", putinTokens);
		printk(KERN_INFO "tokens: %d", p_Session->tokenBucket.tokens);

		if (p_Packet->packetSize <= p_Session->tokenBucket.tokens)
		{
			p_Session->tokenBucket.tokens -= p_Packet->packetSize;
			return PKT_PASS;
		}
		else
		{
			printk(KERN_INFO "drop appID:%d packet", p_Session->appID);
			return PKT_DROP;
		}
	}
	else
	{
		return PKT_PASS;
	}
}

static void BandwidthManage(GlobalStruct *p_gstruct)
{
	int i, level;
	int appID, appConfigID;
	int tmpPermit, finalPermit;
	struct timeval	timeVal;
	Packet			*p_Packet = NULL;
	AppConfig		*p_AppConfig = NULL;
	Policy			*p_Policy = NULL;

	do_gettimeofday(&timeVal);

	appID		= p_gstruct->packet.p_Session->appID;
	level		= p_gstruct->packet.userLevel;
	p_Packet	= &(p_gstruct->packet);
	p_Policy	= &(p_gstruct->policyTable[level]);

	finalPermit = PKT_PASS;		/* default pass */

	for (i = 0; i < p_Policy->appConfigNum; i++)
	{
		appConfigID = p_Policy->appConfigID[i];
		p_AppConfig = &(p_gstruct->appConfigTable[appConfigID]);

		if (p_AppConfig->appID == appID)
		{
			tmpPermit = TokenBucketAlg(p_Packet, &timeVal, p_AppConfig);
			finalPermit = tmpPermit && finalPermit;
			if (finalPermit == PKT_DROP)
			{
				p_Packet->permitted2pass = PKT_DROP;
				return;
			}
		}
	}

	p_Packet->permitted2pass = PKT_PASS;
}

int TTCN_BandwidthManage(int res, GlobalStruct *p_gstruct)
{
	struct timeval timeVal;
	unsigned long long ms;

	if (res == NO_PROCESS)
	{
		return NO_PROCESS;
	}

	if (p_gstruct->packet.p_Session->appID != UNDEFINED_APP)
	{
		BandwidthManage(p_gstruct);
	}

	do_gettimeofday(&timeVal);
	ms = ((unsigned long long)(timeVal.tv_sec))*1000+(timeVal.tv_usec)/1000;
	p_gstruct->packet.p_Session->lastPktTime = ms;

	return NO_PROCESS;
}
