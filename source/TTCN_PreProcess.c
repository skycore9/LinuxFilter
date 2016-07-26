/*
 * Name:	TTCN_PreProcess.c
 *
 * Author:	Feng Wei
 *
 * Date:	2015-02-16
 */

#include <linux/ip.h>
#include <linux/tcp.h>
#include <linux/udp.h>
#include <linux/skbuff.h>
#include "GlobalStruct.h"

int TTCN_PreProcess(struct sk_buff *skb, const struct net_device *in, 
		GlobalStruct *p_gstruct)
{
	struct iphdr *p_IPHead		= NULL;
	struct tcphdr *p_TcpHead	= NULL;
	struct udphdr *p_UdpHead	= NULL;

	p_gstruct->packet.inEthNum	= DEFAULT_IN_ETH;
	p_gstruct->packet.p_Session	= NULL;
	p_gstruct->packet.permitted2pass = PKT_PASS;

	if (in != NULL)
	{
		p_gstruct->packet.inEthNum = in->name[3];
	}

	p_IPHead = (struct iphdr *)skb_network_header(skb);

	p_gstruct->packet.fiveGroup.ip_src	= p_IPHead->saddr;
	p_gstruct->packet.fiveGroup.ip_dst	= p_IPHead->daddr;
	p_gstruct->packet.fiveGroup.ip_p	= p_IPHead->protocol;

	switch (p_IPHead->protocol)
	{
		case TCP_PROTO:
			p_TcpHead = (struct tcphdr *)(skb->data + 4*(p_IPHead->ihl));

			p_gstruct->packet.p_l4hdr = p_TcpHead;
			p_gstruct->packet.fiveGroup.th_sport = p_TcpHead->source;
			p_gstruct->packet.fiveGroup.th_dport = p_TcpHead->dest;

			p_gstruct->packet.p_Payload = 
				( (unsigned char *)p_TcpHead) + 4*(p_TcpHead->doff);

			break;

		case UDP_PROTO:
			p_UdpHead = (struct udphdr *)(skb->data + 4*(p_IPHead->ihl));

			p_gstruct->packet.p_l4hdr = p_UdpHead;
			p_gstruct->packet.fiveGroup.th_sport = p_UdpHead->source;
			p_gstruct->packet.fiveGroup.th_dport = p_UdpHead->dest;

			p_gstruct->packet.p_Payload = ( (unsigned char *)p_UdpHead) + 8;

			break;

		default:
			break;
	}

	p_gstruct->packet.packetSize = skb->len;

	return SESSION_MANAGE;
}
