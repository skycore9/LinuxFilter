/*
 * Name:	TTCN_SessionManage.c
 *
 * Author:	Feng Wei
 *
 * Date:	2015-02-16
 */

#include <linux/slab.h>
#include <linux/time.h>
#include <linux/tcp.h>
#include "GlobalStruct.h"

static int HashFive(FiveGroup *p_FiveGroup)
{
	unsigned int sum		= 0;
	unsigned int destPort	= 0;

	destPort = p_FiveGroup->th_dport;
	sum = p_FiveGroup->ip_p;
	sum += (destPort << 16) + p_FiveGroup->th_sport;
	sum += p_FiveGroup->ip_src + p_FiveGroup->ip_dst;

	return (sum & SESSION_TABLE_HASHCOVER);
}

static void ConvertFive(Packet *p_Packet, FiveGroup *p_SessionFive)
{
	if (p_Packet->inEthNum == DEFAULT_EX_ETH)
	{
		p_SessionFive->ip_src	= p_Packet->fiveGroup.ip_dst;
		p_SessionFive->ip_dst	= p_Packet->fiveGroup.ip_src;
		p_SessionFive->th_sport	= p_Packet->fiveGroup.th_dport;
		p_SessionFive->th_dport	= p_Packet->fiveGroup.th_sport;
		p_SessionFive->ip_p		= p_Packet->fiveGroup.ip_p;
	}
	else
	{
		*p_SessionFive = p_Packet->fiveGroup;
	}
}

static int RightSession(FiveGroup *p_FiveGroup, SessionHashNode *p_Session)
{
	if ( (p_FiveGroup->ip_p == p_Session->fiveGroup.ip_p) &&
		 (p_FiveGroup->ip_src == p_Session->fiveGroup.ip_src) &&
		 (p_FiveGroup->ip_dst == p_Session->fiveGroup.ip_dst) &&
		 (p_FiveGroup->th_sport == p_Session->fiveGroup.th_sport) &&
		 (p_FiveGroup->th_dport	== p_Session->fiveGroup.th_dport))
	{
		return R_SESSION;
	}
	else
	{
		return F_SESSION;
	}
}

static void InsertSessionNode(SessionHashNode *p_SessionNode, 
		SessionHashNode **p_ListHead)
{
	p_SessionNode->next = *p_ListHead;
	if (*p_ListHead != NULL)
	{
		(*p_ListHead)->pre = p_SessionNode;
	}
	*p_ListHead = p_SessionNode;

	p_SessionNode->pre = NULL;
}



static SessionHashNode *SearchSessionLink(FiveGroup *p_SessionFive, 
		SessionHashNode *sessionLink)
{
	SessionHashNode *p_CurrentNode = NULL;

	p_CurrentNode = sessionLink;
	while (p_CurrentNode != NULL)
	{
		if(RightSession(p_SessionFive, p_CurrentNode) == R_SESSION)
		{
			break;
		}
		p_CurrentNode = p_CurrentNode->next;
	}

	return p_CurrentNode;
}

int TTCN_SessionManage(int res, GlobalStruct *p_gstruct)
{
	struct timeval		timeVal;
	unsigned long long	msecs;
	FiveGroup			sessionFive;
	SessionHashNode		*p_SessionNode;
	SessionHashNode		*sessionLink;
	SessionHashNode		**p_ListHead;

	if (!(p_gstruct->packet.fiveGroup.ip_p == TCP_PROTO || 
				p_gstruct->packet.fiveGroup.ip_p == UDP_PROTO))
	{
		return NO_PROCESS;
	}

	do_gettimeofday(&timeVal);
	msecs = ((unsigned long long)timeVal.tv_sec)*1000 + timeVal.tv_usec/1000;

	ConvertFive(&(p_gstruct->packet), &sessionFive);
	sessionLink = p_gstruct->sessionHashTable[HashFive(&sessionFive)];
	p_ListHead = &(p_gstruct->sessionHashTable[HashFive(&sessionFive)]);
	p_SessionNode = SearchSessionLink(&sessionFive, sessionLink);

	if (p_SessionNode == NULL)
	{
		if (p_gstruct->packet.fiveGroup.ip_p == TCP_PROTO &&
				((struct tcphdr *)(p_gstruct->packet.p_l4hdr))->syn == 0)
		{
			return NO_PROCESS;
		}

		p_SessionNode = 
			(SessionHashNode *)kmalloc(sizeof(SessionHashNode), GFP_ATOMIC);

		if (p_SessionNode != NULL)
		{
			p_SessionNode->pre	= NULL;
			p_SessionNode->next	= NULL;
			p_SessionNode->appID		= UNDEFINED_APP;
			p_SessionNode->fiveGroup	= sessionFive;
			p_SessionNode->packets		= 1;
			p_SessionNode->bytes		= p_gstruct->packet.packetSize;
			p_SessionNode->begin		= msecs;
			p_SessionNode->lastPktTime	= msecs;
			p_SessionNode->analysis		= NEED_ANALYSIS;
			p_SessionNode->status		= SESSION_ON;
			p_SessionNode->tokenBucket.bucketSize = BUCKET_SIZE;
			p_SessionNode->tokenBucket.tokens = 0;

			InsertSessionNode(p_SessionNode, p_ListHead);
		}
		else
		{
			printk(KERN_INFO "TTCN_SessionManage: Memory Alloc Error");
			return NO_PROCESS;
		}
	}
	else
	{
		p_SessionNode->packets++;
		p_SessionNode->bytes += p_gstruct->packet.packetSize;
		//p_SessionNode->lastPktTime = msecs;

		if (p_SessionNode->packets > MAX_ANALYSIS_PACKETS)
		{
			p_SessionNode->analysis = NO_MORE_ANALYSIS;
		}

		if (p_SessionNode->fiveGroup.ip_p == TCP_PROTO)
		{
			if ( ((struct tcphdr *)(p_gstruct->packet.p_l4hdr))->rst ||
					((struct tcphdr *)(p_gstruct->packet.p_l4hdr))->fin)
			{
				p_SessionNode->status = SESSION_OFF;
			}
		}
	}

	p_gstruct->packet.p_Session = p_SessionNode;

	return USER_LEVEL;
}
