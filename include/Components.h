/*
 * Name:	Components.h
 *
 * Author:	Feng Wei
 *
 * Date:	2015-02-13
 *
 * Description:	LinuxFilter's components.
 */

#ifndef COMPONENT_H
#define COMPONENT_H

#include <linux/skbuff.h>
#include <linux/netdevice.h>
#include "GlobalStruct.h"

extern int TTCN_PreProcess(struct sk_buff *skb, const struct net_device *in,
		GlobalStruct *p_GlobalStruct);

extern int TTCN_SessionManage(int res, GlobalStruct *p_GlobalStruct);

extern int TTCN_UserLevel(int res, GlobalStruct *p_GlobalStruct);

extern int TTCN_AppDetect(int res, GlobalStruct *p_GlobalStruct);

extern int TTCN_BandwidthManage(int res, GlobalStruct *p_GlobalStruct);

#endif
