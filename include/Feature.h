/*
 * Name:	Feature.h
 *
 * Author:	Feng Wei
 *
 * Date:	2015-02-14
 *
 * Description: some structs about applications'feature
 */

#ifndef FEATURE_H
#define FEATURE_H

#define DETECT_IT				1
#define NO_DETECT				0
#define APP_HIT                 1
#define APP_MISS                0
#define FEATURE_HIT             1
#define FEATURE_MISS            0
#define UNDEFINED_APP			0
#define MAX_APP_NUM             1024
#define MAX_PORT_NUM			65536
#define SIG_LEN                 14
#define HTTP_PORT				80


typedef struct FeatureNode
{
	unsigned char			sigs[SIG_LEN];
	unsigned char			mask[SIG_LEN];
	unsigned char			proto;
	struct FeatureNode		*next;
}FeatureNode;

typedef struct FeatureHead
{
	int				appID;
	char			detectFlag;
	FeatureNode		*features;
}FeatureHead;

typedef struct FeatureTable
{
	int				tport[MAX_PORT_NUM];
	int				uport[MAX_PORT_NUM];
	FeatureHead		featureHead[MAX_APP_NUM];
	int				appNum;
}FeatureTable;

#endif
