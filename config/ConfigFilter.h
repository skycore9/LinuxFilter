/*
 * Name:	ConfigFilter.h
 *
 * Author:	Feng Wei
 *
 * Date:	2015-02-10
 *
 * Description:
 *
 * Initialize or configure feature  data.
 */

#ifndef CONFIGFILTER_H
#define CONFIGFILTER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <linux/netlink.h>
#include <unistd.h>

#define COMMAND_NAME_LEN		16
#define PARAMETER_LEN			128
#define MAX_PAYLOAD_LEN			160
#define SIG_LEN					14
#define TCP_PROTO				6
#define UDP_PROTO				17

typedef struct Command
{
	char 			mainCmdName[COMMAND_NAME_LEN];
	char			subCmdName[COMMAND_NAME_LEN];
	unsigned char	param[PARAMETER_LEN];
}Command;

typedef struct FilterMesg
{
	struct nlmsghdr hdr;
	unsigned char data[MAX_PAYLOAD_LEN];
}FilterMesg;

typedef struct UserConfig 
{
	unsigned int	ip;
	int				level;
}UserConfig;

typedef struct FeatureConfig
{
	int				appID;
	unsigned char	sigs[SIG_LEN];
	unsigned char	mask[SIG_LEN];
	unsigned char	proto;
}FeatureConfig;

typedef struct AppConfig
{
	int appConfigID;
	int appID;
	int bandwidth;
	int beginTime; /* unit is minute */
	int endTime;
}AppConfig;

typedef struct PolicyConfig
{
	int level;
	int appConfigID;
}PolicyConfig;

#endif
