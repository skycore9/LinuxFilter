/*
 * Name:	ConfigFilter.c
 *
 * Author:	Feng Wei
 *
 * Date:	2015-02-01
 *
 * Description: 
 *
 * Get configure information from userspace and send it to kernel.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mysql/mysql.h>
#include "ConfigFilter.h"
#include "ParseFeature.h"

#define INIT_SYS				1
#define USER_CONFIG				2
#define FEATURE_CONFIG			3
#define APP_CONFIG				4
#define POLICY_CONFIG			5
#define MAX_PAYLOAD_LEN			160
#define NETLINK_LINUXFILTER		31

const char *INIT_CMD		= "init";
const char *USER_CMD		= "user";
const char *FEATURE_CMD		= "load";
const char *APPCONFIG_CMD	= "appconfig";
const char *POLICY_CMD		= "policy";
const char *APP_CMD			= "app";
const char *WRITE_DB		= "write_db";

const char *DATABASE		= "filter_config";
const char *FETCH_USERLEVEL = "select * from T_UserLevel";
const char *FETCH_APPCONFIG	= "select * from T_AppConfig";
const char *FETCH_POLICY	= "select * from R_LevelAppConfig";
const char *FEATURE_FILE	= "signature.xml";

static int InitSocket(struct sockaddr_nl *p_srcaddr)
{
	int	sock;

	p_srcaddr->nl_family	= AF_NETLINK;
	p_srcaddr->nl_pad		= 0;
	p_srcaddr->nl_pid		= getpid();
	p_srcaddr->nl_groups	= 0;

	if ( (sock = socket(AF_NETLINK, SOCK_DGRAM, NETLINK_LINUXFILTER)) < 0)
	{
		return -1;
	}

	if (bind(sock,(struct sockaddr *)p_srcaddr,sizeof(struct sockaddr_nl)) < 0)
	{
		return -1;
	}

	return sock;
}

static MYSQL *InitMysql()
{
	MYSQL *mysql;

	if ( (mysql = mysql_init(NULL)) == NULL)
	{
		return NULL;
	}

	if (mysql_real_connect(mysql, "localhost", NULL, NULL, DATABASE, 0, NULL, 0) == NULL)
	{
		return NULL;
	}

	return mysql;
}

static int ParseCommand(int argc, char *argv[])
{
	if (strcmp(argv[1], INIT_CMD) == 0)
	{
		return INIT_SYS;
	}
	else if (strcmp(argv[1], USER_CMD) == 0)
	{
		return USER_CONFIG;
	}
	else if (strcmp(argv[1], FEATURE_CMD) == 0)
	{
		return FEATURE_CONFIG;
	}
	else if (strcmp(argv[1], APPCONFIG_CMD) == 0)
	{
		return APP_CONFIG;
	}
	else if (strcmp(argv[1], POLICY_CMD) == 0)
	{
		return POLICY_CONFIG;
	}

	return 0;
}

static void ProcessRow(MYSQL_ROW row, void *structure, int type)
{
	UserConfig	*p_UserConfig;
	AppConfig	*p_AppConfig;
	PolicyConfig *p_PolicyConfig;

	switch (type)
	{
		case USER_CONFIG:
			p_UserConfig = (UserConfig *)structure;
			inet_aton(row[0], (struct in_addr *)&(p_UserConfig->ip));
			p_UserConfig->level	= strtol(row[1], NULL, 10);
			break;
		case APP_CONFIG:
			p_AppConfig = (AppConfig *)structure;
			p_AppConfig->appConfigID = strtol(row[0], NULL, 10);
			p_AppConfig->appID = strtol(row[1], NULL, 10);
			p_AppConfig->bandwidth = strtol(row[2], NULL, 10);
			p_AppConfig->beginTime = strtol(row[3], NULL, 10);
			p_AppConfig->endTime = strtol(row[4], NULL, 10);
			break;
		case POLICY_CONFIG:
			p_PolicyConfig = (PolicyConfig *)structure;
			p_PolicyConfig->level = strtol(row[0], NULL, 10);
			p_PolicyConfig->appConfigID = strtol(row[1], NULL, 10);
			break;
		default:
			break;
	}
}

static void ConfigUser(MYSQL *mysql, int sock, struct sockaddr_nl *p_dstaddr)
{
	MYSQL_RES		*result;
	MYSQL_ROW		row;
	Command			command;
	UserConfig		userConfig;
	FilterMesg		msg;
	int numFields;

	mysql_query(mysql, FETCH_USERLEVEL);
	result		= mysql_store_result(mysql);
	numFields	= mysql_num_fields(result);

	msg.hdr.nlmsg_len	= sizeof(msg);
	msg.hdr.nlmsg_type	= 0;
	msg.hdr.nlmsg_flags	= 0;
	msg.hdr.nlmsg_seq	= 0;
	msg.hdr.nlmsg_pid	= getpid();

	strcpy(command.mainCmdName, USER_CMD);
	while ( (row = mysql_fetch_row(result)) != NULL)
	{
		ProcessRow(row, &userConfig, USER_CONFIG);
		memcpy(command.param, &userConfig, sizeof(userConfig));
		memcpy(msg.data, &command, sizeof(command));

		sendto(sock, &msg, sizeof(msg), 0, (struct sockaddr *)p_dstaddr, 
				sizeof(struct sockaddr_nl));
	}
}

static void LoadAppConfig(MYSQL *mysql, int sock, struct sockaddr_nl *p_dstaddr)
{
	MYSQL_RES	*result;
	MYSQL_ROW	row;
	Command		command;
	AppConfig	appConfig;
	FilterMesg	msg;
	int numFields;
	int i;

	strcpy(command.mainCmdName, APPCONFIG_CMD);
	msg.hdr.nlmsg_len	= sizeof(msg);
	msg.hdr.nlmsg_type	= 0;
	msg.hdr.nlmsg_flags	= 0;
	msg.hdr.nlmsg_seq	= 0;
	msg.hdr.nlmsg_pid	= getpid();

	mysql_query(mysql, FETCH_APPCONFIG);
	result = mysql_store_result(mysql);
	numFields = mysql_num_fields(result);

	while ( (row = mysql_fetch_row(result)) != NULL)
	{
		ProcessRow(row, &appConfig, APP_CONFIG);
		memcpy(command.param, &appConfig, sizeof(appConfig));
		memcpy(msg.data, &command, sizeof(command));
		sendto(sock, &msg, sizeof(msg), 0, (struct sockaddr *)p_dstaddr, 
				sizeof(struct sockaddr_nl));
	}
}

static void ConfigPolicy(MYSQL *mysql, int sock, struct sockaddr_nl *p_dstaddr)
{
	MYSQL_RES		*result;
	MYSQL_ROW		row;
	Command			command;
	PolicyConfig	policyConfig;
	FilterMesg		msg;
	int numFields;

	mysql_query(mysql, FETCH_POLICY);
	result		= mysql_store_result(mysql);
	numFields	= mysql_num_fields(result);

	msg.hdr.nlmsg_len	= sizeof(msg);
	msg.hdr.nlmsg_type	= 0;
	msg.hdr.nlmsg_flags	= 0;
	msg.hdr.nlmsg_seq	= 0;
	msg.hdr.nlmsg_pid	= getpid();

	strcpy(command.mainCmdName, POLICY_CMD);
	while ( (row = mysql_fetch_row(result)) != NULL)
	{
		ProcessRow(row, &policyConfig, POLICY_CONFIG);
		memcpy(command.param, &policyConfig, sizeof(policyConfig));
		memcpy(msg.data, &command, sizeof(command));

		sendto(sock, &msg, sizeof(msg), 0, (struct sockaddr *)p_dstaddr, 
				sizeof(struct sockaddr_nl));
	}
}

void DoLoadFeature(FILE *fp, int sock, struct sockaddr_nl *p_dstaddr)
{
	char buf[LINE_BUF_LEN];
	int i, len;
	int appID;
	Command command;
	FeatureConfig fconfig;
	FilterMesg msg;

	msg.hdr.nlmsg_len = sizeof(msg);
	msg.hdr.nlmsg_type = 0;
	msg.hdr.nlmsg_flags	= 0;
	msg.hdr.nlmsg_seq = 0;
	msg.hdr.nlmsg_pid = getpid();

	strcpy(command.mainCmdName, FEATURE_CMD);

	while ( (len = readLine(fp, buf)) != EOF)
	{
		if (appStart(buf))
		{
			appID = getAppID(buf, len);
			len = readLine(fp, buf);
			while (featureStart(buf))
			{
				fconfig.appID = appID;
				getFeature(fp, &fconfig);
				memcpy(command.param, &fconfig, sizeof(fconfig));
				memcpy(msg.data, &command, sizeof(command));
				sendto(sock, &msg, sizeof(msg), 0, 
						(struct sockaddr *)p_dstaddr, 
						sizeof(struct sockaddr_nl));
				len = readLine(fp, buf);
			}
		}
	}
}

void LoadFeature(int sock, struct sockaddr_nl *p_dstaddr)
{
	FILE *features;

	if ( (features = fopen("signature.xml", "r")) == NULL)
	{
		printf("Cann't open feature file\n");
		return;
	}

	DoLoadFeature(features, sock, p_dstaddr);
	fclose(features);
}

void ConfigFilter(int argc, char *argv[], MYSQL *conn, int sock, 
		struct sockaddr_nl *p_dstaddr)
{
	int command;

	command = ParseCommand(argc, argv);
	switch (command)
	{
		case INIT_SYS:
			break;
		case USER_CONFIG:
			ConfigUser(conn, sock, p_dstaddr);
			break;
		case FEATURE_CONFIG:
			LoadFeature(sock, p_dstaddr);
			break;
		case APP_CONFIG:
			LoadAppConfig(conn, sock, p_dstaddr);
			break;
		case POLICY_CONFIG:
			ConfigPolicy(conn, sock, p_dstaddr);
		default:
			break;
	}
}

int main(int argc, char *argv[])
{
	int		sock;
	MYSQL	*mysql;
	struct sockaddr_nl	srcaddr;
	struct sockaddr_nl	dstaddr;

	if ( (sock = InitSocket(&srcaddr)) < 0)
	{
		printf("InitSocket error\n");
		return 0;
	}

	if ( (mysql = InitMysql()) == NULL)
	{
		printf("InitMysql error\n");
		return 0;
	}

	dstaddr.nl_family	= AF_NETLINK;
	dstaddr.nl_pad		= 0;
	dstaddr.nl_pid		= 0;
	dstaddr.nl_groups	= 0;

	ConfigFilter(argc, argv, mysql, sock, &dstaddr);

	close(sock);
	mysql_close(mysql);

	return 0;
}
