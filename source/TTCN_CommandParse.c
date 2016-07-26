/*
 * Name:	TTCN_CommandParse.c
 *
 * Author:	Feng Wei
 *
 * Date:	2015-02-16
 */

#include <linux/skbuff.h>
#include <linux/netlink.h>
#include <net/sock.h>
#include <linux/string.h>
#include "Command.h"
#include "GlobalStruct.h"

extern GlobalStruct		gstruct;
extern struct sock		*nlsock;

const char *INIT_CMD		= "init";
const char *USER_CMD		= "user";
const char *FEATURE_CMD		= "load";
const char *APP_CMD			= "app";
const char *APPCONFIG_CMD	= "appconfig";
const char *POLICY_CMD		= "policy";
const char *WRITEDB_CMD		= "write_db";

static void ConfigUser(UserConfig *p_User, UserTable *p_UserTable)
{
	int i;

	if (p_UserTable->userNum >= MAX_USER_NUM)
	{
		return;
	}

	i = p_UserTable->userNum - 1;
	while (i >= 0 && p_UserTable->user[i].ip > p_User->ip)
	{
		p_UserTable->user[i+1] = p_UserTable->user[i];
		i--;
	}

	p_UserTable->user[i+1].ip		= p_User->ip;
	p_UserTable->user[i+1].level	= p_User->level;
	p_UserTable->userNum++;
}

static void ConfigFeature(FeatureConfig *p_Feature, FeatureTable *p_FeatureTable)
{
 	FeatureNode *p_FeatureNode;
	int appID, appIn;
	int indx;

	p_FeatureNode = (FeatureNode *)kmalloc(sizeof(FeatureNode), GFP_ATOMIC);
	appID = p_Feature->appID;

	if (p_FeatureNode != NULL)
	{
		memcpy(p_FeatureNode->sigs, p_Feature->sigs, SIG_LEN);
		memcpy(p_FeatureNode->mask, p_Feature->mask, SIG_LEN);
		p_FeatureNode->proto = p_Feature->proto;
		p_FeatureNode->next = NULL;
	}
	else
	{
		printk(KERN_INFO "LoadSigFeature: NULL Point Error");
		return;
	}

	appIn = 0;
	for (indx = 0; indx < p_FeatureTable->appNum; indx++)
	{
		if (p_FeatureTable->featureHead[indx].appID == appID)
		{
			appIn = 1;
			break;
		}
	}

	if (appIn == 0)
	{
		if (p_FeatureTable->appNum > MAX_APP_NUM)
		{
			return;
		}
		indx = p_FeatureTable->appNum;
		p_FeatureTable->featureHead[indx].appID = appID;
		p_FeatureTable->appNum++;
	}

	p_FeatureNode->next = p_FeatureTable->featureHead[indx].features;
	p_FeatureTable->featureHead[indx].features = p_FeatureNode;
}

static void LoadAppConfig(AppConfig *p_AppConfig, AppConfig *appConfigTable)
{
	int indx;

	indx = p_AppConfig->appConfigID;
	if (indx >= 0 && indx < MAX_APPCONFIG_NUM)
	{
		appConfigTable[indx].appConfigID = indx;
		appConfigTable[indx].appID = p_AppConfig->appID;
		appConfigTable[indx].bandwidth = p_AppConfig->bandwidth;
		appConfigTable[indx].beginTime = p_AppConfig->beginTime;
		appConfigTable[indx].endTime = p_AppConfig->endTime;
	}
}

static void LoadPolicyConfig(PolicyConfig *p_PolicyConfig, 
		Policy *policyTable)
{
	int level, indx;
	Policy *p_Policy;

	level = p_PolicyConfig->level;
	if (level >= 0 && level < MAX_LEVEL_NUM)
	{
		p_Policy = &(policyTable[level]);
	}
	indx = p_Policy->appConfigNum;
	if (indx >= 0 && indx < MAX_APPCONFIG_NUM)
	{
		p_Policy->appConfigID[indx] = p_PolicyConfig->appConfigID;
		p_Policy->appConfigNum++;
	}
}

static void DeleteSessionNode(SessionHashNode *p_SessionNode, 
		SessionHashNode **p_ListHead)
{
	if (p_SessionNode->pre == NULL)
		*p_ListHead = p_SessionNode->next;
	else
		p_SessionNode->pre->next = p_SessionNode->next;
	
	if (p_SessionNode->next != NULL)
		p_SessionNode->next->pre = p_SessionNode->pre;

	kfree(p_SessionNode);
}

void GetSessionData(SessionHashNode *sessionHashTable[], SessionData *p_Data)
{
	int i;
	struct timeval timeVal;
	unsigned long long msecs;
	SessionInfo		*p_SessionInfo = NULL;
	SessionHashNode	*p_CurrentNode = NULL;
	SessionHashNode *p_NextNode = NULL;

	do_gettimeofday(&timeVal);
	msecs = ((unsigned long long)timeVal.tv_sec)*1000 + timeVal.tv_usec / 1000;

	p_Data->sessionNum = 0;
	for (i = 0; i < SESSION_TABLE_SIZE; i++)
	{
		p_CurrentNode = sessionHashTable[i];

		while (p_CurrentNode != NULL)
		{
			p_SessionInfo = &(p_Data->sessionInfo[p_Data->sessionNum]);

			p_SessionInfo->inIP		= p_CurrentNode->fiveGroup.ip_src;
			p_SessionInfo->exIP		= p_CurrentNode->fiveGroup.ip_dst;
			p_SessionInfo->inPort	= p_CurrentNode->fiveGroup.th_sport;
			p_SessionInfo->exPort	= p_CurrentNode->fiveGroup.th_dport;
			p_SessionInfo->protocol	= p_CurrentNode->fiveGroup.ip_p;
			p_SessionInfo->begin	= p_CurrentNode->begin;
			p_SessionInfo->end		= msecs;
			p_SessionInfo->appID	= p_CurrentNode->appID;
			p_SessionInfo->pakcets	= p_CurrentNode->packets;
			p_SessionInfo->bytes	= p_CurrentNode->bytes;

			p_Data->sessionNum++;
			p_NextNode = p_CurrentNode->next;

			/* reset packet/byte counter */
			p_CurrentNode->packets = 0;
			p_CurrentNode->bytes = 0;

			if (p_CurrentNode->fiveGroup.ip_p == UDP_PROTO)
			{
				if (msecs - p_CurrentNode->lastPktTime > UDP_SESSION_MAX_LIVE)
				{
					DeleteSessionNode(p_CurrentNode, &(sessionHashTable[i]));
				}
			}
			else if (p_CurrentNode->fiveGroup.ip_p == TCP_PROTO)
			{
				if (p_CurrentNode->status == SESSION_OFF)
				{
					DeleteSessionNode(p_CurrentNode, &(sessionHashTable[i]));
				}
			}

			p_CurrentNode = p_NextNode;
		}
	}
}

void TTCN_CommandParse(struct sk_buff *skb)
{
	Command				*command = NULL;
	UserConfig			*p_User = NULL;
	FeatureConfig		*p_Feature = NULL;
	AppConfig			*p_AppConfig = NULL;
	PolicyConfig		*p_PolicyConfig = NULL;
	struct nlmsghdr		*nlhdr_in = NULL;
	struct nlmsghdr		*nlhdr_out = NULL;
	struct sk_buff		*data_skb = NULL;
	int data_size;

	nlhdr_in = (struct nlmsghdr *)(skb->data);
	command  = (Command *)nlmsg_data(nlhdr_in);

	if (strcmp(command->mainCmdName, USER_CMD) == 0)
	{
		p_User = (UserConfig *)(command->param);
		ConfigUser(p_User, &(gstruct.userTable));
	}
	else if (strcmp(command->mainCmdName, FEATURE_CMD) == 0)
	{
		p_Feature = (FeatureConfig *)(command->param);
		ConfigFeature(p_Feature, &(gstruct.featureTable));
	}
	else if (strcmp(command->mainCmdName, APPCONFIG_CMD) == 0)
	{
		p_AppConfig = (AppConfig *)(command->param);
		LoadAppConfig(p_AppConfig, gstruct.appConfigTable);
	}
	else if (strcmp(command->mainCmdName, POLICY_CMD) == 0)
	{
		p_PolicyConfig = (PolicyConfig *)(command->param);
		LoadPolicyConfig(p_PolicyConfig, gstruct.policyTable);
	}
	else if (strcmp(command->mainCmdName, WRITEDB_CMD) == 0)
	{
		GetSessionData(gstruct.sessionHashTable, &(gstruct.sessionData));
		data_size = sizeof(SessionData);
		data_skb = nlmsg_new(data_size, GFP_ATOMIC);
		nlhdr_out = nlmsg_put(data_skb, 0, 0, NLMSG_DONE, data_size, 0); 
		memcpy(nlmsg_data(nlhdr_out), &(gstruct.sessionData), data_size);
		NETLINK_CB(data_skb).dst_group = 0;
		nlmsg_unicast(nlsock, data_skb, nlhdr_in->nlmsg_pid);
	}
}
