/*
 * Name:	TTCN_AppDetect.c
 *
 * Author:	Feng Wei
 *
 * Date;	2015-02-16
 */

#include <linux/stddef.h>
#include <linux/kernel.h>
#include "GlobalStruct.h"

static int MatchFeature(unsigned char *payload, FeatureNode *p_Feature)
{
	int i = 0;

	for (i=0; i<SIG_LEN; i++)
	{
		if( ( (payload[i] ^ p_Feature->sigs[i]) & p_Feature->mask[i]) != 0)
		{
			return FEATURE_MISS;
		}
	}

	return FEATURE_HIT;
}

static int MatchApp(unsigned char *playload, FeatureNode *features)
{
	FeatureNode *p_CurrentFeature = features;

	while (p_CurrentFeature != NULL)
	{
		if (MatchFeature(playload, p_CurrentFeature) == FEATURE_HIT)
		{
			return APP_HIT;
		}

		p_CurrentFeature = p_CurrentFeature->next;
	}

	return APP_MISS;
}

static void AppSigDetect(GlobalStruct *p_gstruct)
{
	int				i;
	int				appID;
	unsigned char	*payload = NULL;
	FeatureHead		*p_FeatureHead = NULL;
	FeatureNode		*features = NULL;
	SessionHashNode	*p_Session = NULL;

	payload			= p_gstruct->packet.p_Payload;
	p_FeatureHead	= p_gstruct->featureTable.featureHead;
	p_Session		= p_gstruct->packet.p_Session;

	for (i = 0; i < p_gstruct->featureTable.appNum; i++)
	{
		if (p_FeatureHead[i].detectFlag == DETECT_IT)
		{
			appID = p_FeatureHead[i].appID;
			features = p_FeatureHead[i].features;
			
			if (MatchApp(payload, features) == APP_HIT)
			{
				p_Session->appID = appID;
				p_Session->analysis = NO_MORE_ANALYSIS;
				break;
			}
		}
	}
}

int TTCN_AppDetect(int res, GlobalStruct *p_gstruct)
{
	if (res == NO_PROCESS)
	{
		return NO_PROCESS;
	}

	if (p_gstruct->packet.p_Session->analysis == NEED_ANALYSIS)
	{
		AppSigDetect(p_gstruct);
	}

	return BANDWIDTH_MANAGE;
}
