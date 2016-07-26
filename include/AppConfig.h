/*
 * Name:	AppConfig.h
 *
 * Author:	Feng Wei
 *
 * Date:	2015-02-13
 *
 * Description:	Configuration of policy.
 */

#ifndef APPCONFIG_H
#define APPCONFIG_H

#define PKT_PASS				-1
#define PKT_DROP				0
#define APPCONFIG_HIT			1
#define APPCONFIG_MISS			0
#define IN_RANGE				1
#define OUT_RANGE				0
#define UNDEFINED_APPCONFIG		-1
#define UNDEFINED_TIME			-1
#define MAX_APPCONFIG_NUM		14336
#define MAX_LEVEL_NUM			32

typedef struct AppConfig
{
	int appConfigID;
	int appID;
	int bandwidth;		/* units are KB/sec */
	int beginTime;		/* units are min */
	int endTime;
}AppConfig;

typedef struct Policy
{
	int		appConfigID[MAX_APPCONFIG_NUM];
	int		appConfigNum;
}Policy;

#endif
