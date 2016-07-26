/*
 * Name:	UserTable.h
 *
 * Author:	Feng Wei
 *
 * Date:	2015-02-15
 */

#ifndef USERTABLE_H
#define USERTABLE_H

#define UNDEFINED_USER			-1
#define MAX_USER_NUM			1024

typedef struct UserEntry
{
	unsigned int	ip;
	int 			level;
}UserEntry;

typedef struct UserTable
{
	UserEntry		user[MAX_USER_NUM];
	int				userNum;
}UserTable;

#endif
