/*
 * Name:	TTCN_DispatchLayer.c
 *
 * Author:	Feng Wei
 *
 * Date:	2015-02-16
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include "Components.h"
#include "GlobalStruct.h"

#define NETLINK_LINUXFILTER		31

extern void TTCN_CommandParse(struct sk_buff *skb);

struct sock				*nlsock = NULL;
struct nf_hook_ops		hookops;
GlobalStruct			gstruct;

static void FreeSessionTable(SessionHashNode *sessionHashTable[])
{
	int i;
	SessionHashNode *p_CurrentSession	= NULL;
	SessionHashNode *p_NextSession		= NULL;

	for (i=0; i<SESSION_TABLE_SIZE; i++)
	{
		p_CurrentSession = sessionHashTable[i];
		while (p_CurrentSession != NULL)
		{
			p_NextSession = p_CurrentSession->next;
			kfree(p_CurrentSession);
			p_CurrentSession = p_NextSession;
		}
	}
}

static void FreeFeatureTable(FeatureTable *p_FeatureTable)
{
	int i;
	FeatureNode	*p_CurrentFeature	= NULL;
	FeatureNode	*p_NextFeature		= NULL;

	for (i = 0; i < p_FeatureTable->appNum; i++)
	{
		p_CurrentFeature = p_FeatureTable->featureHead[i].features;
		while (p_CurrentFeature != NULL)
		{
			p_NextFeature = p_CurrentFeature->next;
			kfree(p_CurrentFeature);
			p_CurrentFeature = p_NextFeature;
		}
	}
}

static unsigned int TTCN_Dispatch_Layer(unsigned int hooknum,
		struct sk_buff *skb,
        const struct net_device *in,
		const struct net_device *out,
        int (*okfn)(struct sk_buff *))
{
	int res;

	res = TTCN_PreProcess(skb, in, &gstruct);
	res = TTCN_SessionManage(res, &gstruct);
	res = TTCN_UserLevel(res, &gstruct);
	res = TTCN_AppDetect(res, &gstruct);
	res = TTCN_BandwidthManage(res, &gstruct);

	if (gstruct.packet.permitted2pass == PKT_PASS)
	{
		return NF_ACCEPT;
	}
	else
	{
		return NF_DROP;
	}
}

static int __init TTCN_Dispatch_Layer_init(void)
{
	int i;

	printk(KERN_INFO "LinuxFilter starts...");
	printk(KERN_INFO " ");

	gstruct.userTable.userNum		= 0;
	gstruct.featureTable.appNum		= 0;

	for (i = 0; i < MAX_PORT_NUM; i++)
	{
		gstruct.featureTable.tport[i] = UNDEFINED_APP;
		gstruct.featureTable.uport[i] = UNDEFINED_APP;
	}

	for (i = 0; i < MAX_APP_NUM; i++)
	{
		gstruct.featureTable.featureHead[i].detectFlag = DETECT_IT;
	}

	for (i = 0; i < SESSION_TABLE_SIZE; i++)
	{
		gstruct.sessionHashTable[i] = NULL;
	}

	for (i = 0; i < MAX_LEVEL_NUM; i++)
	{
		gstruct.policyTable[i].appConfigNum = 0;
	}

	nlsock = netlink_kernel_create(&init_net, NETLINK_LINUXFILTER, 0, 
			TTCN_CommandParse, NULL, THIS_MODULE);

    hookops.hook		= TTCN_Dispatch_Layer;
    hookops.hooknum		= NF_INET_FORWARD;
    hookops.pf			= PF_INET;
    hookops.priority	= NF_IP_PRI_FIRST;

    nf_register_hook(&hookops);

    return 0;
}

static void __exit TTCN_Dispatch_Layer_exit(void)
{
	FreeSessionTable(gstruct.sessionHashTable);
	FreeFeatureTable(&(gstruct.featureTable));

	nf_unregister_hook(&hookops);
	netlink_kernel_release(nlsock);

	printk(KERN_INFO " ");
	printk(KERN_INFO "LinuxFilter exits...");
}

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Feng Wei");

module_init(TTCN_Dispatch_Layer_init);
module_exit(TTCN_Dispatch_Layer_exit);
