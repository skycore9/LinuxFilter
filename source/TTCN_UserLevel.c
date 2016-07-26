/*
 * Name:	TTCN_UserLevel.c
 *
 * Name:	Feng Wei
 *
 * Date:	2015-02-16
 */

#include <linux/kernel.h>
#include "GlobalStruct.h"

static int SearchUserTable(unsigned int ip, const UserTable *p_UserTable)
{
	int low = 0;
	int high = p_UserTable->userNum - 1;
	int mid;

	while (low <= high)
	{
		mid = (low + high)/2;

		if (ip < p_UserTable->user[mid].ip)
		{
			high = mid - 1;
		}
		else if (ip == p_UserTable->user[mid].ip)
		{
			return p_UserTable->user[mid].level;
		}
		else
		{
			low = mid + 1;
		}
	}

	return UNDEFINED_USER;
}

int TTCN_UserLevel(int res, GlobalStruct *p_gstruct)
{
	unsigned int ip;

	if (res == NO_PROCESS)
	{
		return NO_PROCESS;
	}

	ip = p_gstruct->packet.p_Session->fiveGroup.ip_src;
	p_gstruct->packet.userLevel = SearchUserTable(ip, &(p_gstruct->userTable));

	return APP_DETECT;
}
