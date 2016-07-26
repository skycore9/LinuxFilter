/*
 * Name:	GlobalStruct.h
 *
 * Author:	Feng Wei
 *
 * Date:	2015-02-15
 *
 * Description: Some About Global Structs
 */

#ifndef GLOBALSTRUCT_H
#define GLOBALSTRUCT_H

#include "Packet.h"
#include "Session.h"
#include "UserTable.h"
#include "Feature.h"
#include "AppConfig.h"
#include "Statistics.h"

#define NO_PROCESS				0
#define SESSION_MANAGE			1
#define USER_LEVEL				2
#define APP_DETECT				3
#define BANDWIDTH_MANAGE		4

typedef struct GlobalStruct
{
	Packet				packet;
	SessionHashNode		*sessionHashTable[SESSION_TABLE_SIZE];
	UserTable			userTable;
	FeatureTable		featureTable;
	AppConfig			appConfigTable[MAX_APPCONFIG_NUM];
	Policy				policyTable[MAX_LEVEL_NUM];
	SessionData			sessionData;
}GlobalStruct;

#endif
