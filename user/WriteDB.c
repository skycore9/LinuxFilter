/*
 * Name:	WriteDB.c
 *
 * Author:	Feng Wei
 *
 * Date:	2015-02-16
 *
 * Description: 
 * Send a message to kernel and get session information from kernel and 
 * write sessioninfo to database.
 */

#include <sys/socket.h>
#include <linux/netlink.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <mysql/mysql.h>

#define MAX_INFO_LOG			16
#define MAX_INFO_NUM			(1<<MAX_INFO_LOG)
#define WRITE_DB_INTVAL			10
#define MAX_STATEMENT_LEN		256
#define CMDNAME_LEN				16
#define PARAMETER_LEN			128
#define MAX_PAYLOAD				256
#define DATABASE				"filter_config"
#define NETLINK_LINUXFILTER		31

typedef struct SessionInfo
{
	unsigned int inIP;
	unsigned int exIP;
	unsigned short inPort;
	unsigned short exPort;
	unsigned char protocol;
	unsigned long long begin;
	unsigned long long end;
	int appID;
	unsigned long long pakcets;
	unsigned long long bytes;
}SessionInfo;

typedef struct SessionData
{
	SessionInfo sessionInfo[MAX_INFO_NUM];
	int sessionNum;
}SessionData;

typedef struct Command
{
	char			mainCmdName[CMDNAME_LEN];
	char			subCmdName[CMDNAME_LEN];
	unsigned char	param[PARAMETER_LEN];
}Command;

void GetSQLStatement(char *query, char *statement, SessionInfo *sessionInfo)
{
	char inIP[INET_ADDRSTRLEN];
	char exIP[INET_ADDRSTRLEN];
	char protocol[8];

	if (sessionInfo != NULL)
	{
		inet_ntop(AF_INET, &(sessionInfo->inIP), inIP, INET_ADDRSTRLEN);
		inet_ntop(AF_INET, &(sessionInfo->exIP), exIP, INET_ADDRSTRLEN);
		switch (sessionInfo->protocol)
		{
			case 6:
				strcpy(protocol, "TCP");
				break;
			case 17:
				strcpy(protocol, "UDP");
				break;
			default:
				protocol[0] = '\0';
				break;
		}


		snprintf(query, MAX_STATEMENT_LEN, statement, 
				 inIP, 
				 exIP, 
				 ntohs(sessionInfo->inPort), 
				 ntohs(sessionInfo->exPort), 
				 protocol,
				 sessionInfo->begin, 
				 sessionInfo->end, 
				 sessionInfo->appID, 
				 sessionInfo->pakcets, 
				 sessionInfo->bytes);
	}
}

void WriteDB(MYSQL *conn, SessionData *sessionData)
{
	char *statement = "INSERT INTO T_SessionInfo VALUES('%s', '%s', %d, %d, '%s', %llu, %llu, %d, %llu, %llu)";
	char query[MAX_STATEMENT_LEN];
	int i;

	
	for (i = 0; i < sessionData->sessionNum; i++)
	{
		GetSQLStatement(query, statement, &(sessionData->sessionInfo[i]));
		mysql_real_query(conn, query, strlen(query));
	}
	printf("session number %d\n", sessionData->sessionNum);
}

int main(int argc, char *argv[])
{
	int sockd;
	struct iovec iovToKernel;
	struct iovec iovFromKernel;
	struct msghdr msgToKernelHdr;
	struct msghdr msgFromKernelHdr;
	struct sockaddr_nl srcAddr;
	struct sockaddr_nl dstAddr;
	struct nlmsghdr *nlmsgToKernelHdr;
	struct nlmsghdr *nlmsgFromKernelHdr;

	Command cmd;
	SessionData *p_SessionData;

	MYSQL *conn;

	nlmsgToKernelHdr = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
	nlmsgFromKernelHdr = (struct nlmsghdr *)malloc(NLMSG_SPACE(sizeof(SessionData)));

	if (nlmsgToKernelHdr == NULL || nlmsgFromKernelHdr == NULL)
	{
		return -1;
	}

	memset(&srcAddr, 0, sizeof(srcAddr));
	memset(&dstAddr, 0, sizeof(dstAddr));
	memset(nlmsgToKernelHdr, 0, sizeof(struct nlmsghdr));
	memset(nlmsgFromKernelHdr, 0, sizeof(struct nlmsghdr));
	memset(&msgToKernelHdr, 0, sizeof(msgToKernelHdr));
	memset(&msgFromKernelHdr, 0, sizeof(msgFromKernelHdr));

	srcAddr.nl_family = PF_NETLINK;
	srcAddr.nl_pid = getpid();
	dstAddr.nl_family = PF_NETLINK;
	dstAddr.nl_pid = 0;

	if ( (sockd = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_LINUXFILTER)) < 0)
	{
		return -1;
	}
	bind(sockd, (struct sockaddr *)&srcAddr, sizeof(srcAddr));

	if ( (conn = mysql_init(NULL)) == NULL)
	{
		return -1;
	}

	if (mysql_real_connect(conn, "localhost", NULL, NULL, DATABASE, 0, NULL, 0) == NULL)
	{
		return -1;
	}

	nlmsgToKernelHdr->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
	nlmsgToKernelHdr->nlmsg_pid = getpid();

	strcpy(cmd.mainCmdName, "write_db");
	memcpy(NLMSG_DATA(nlmsgToKernelHdr), &cmd, sizeof(cmd));

	iovToKernel.iov_base = (void *)nlmsgToKernelHdr;
	iovToKernel.iov_len = nlmsgToKernelHdr->nlmsg_len;
	msgToKernelHdr.msg_name = (void *)&dstAddr;
	msgToKernelHdr.msg_namelen = sizeof(dstAddr);
	msgToKernelHdr.msg_iov = &iovToKernel;
	msgToKernelHdr.msg_iovlen = 1;

	iovFromKernel.iov_base = (void *)nlmsgFromKernelHdr;
	iovFromKernel.iov_len = NLMSG_SPACE(sizeof(SessionData));
	msgFromKernelHdr.msg_name = (void *)&srcAddr;
	msgFromKernelHdr.msg_namelen = sizeof(srcAddr);
	msgFromKernelHdr.msg_iov = &iovFromKernel;
	msgFromKernelHdr.msg_iovlen = 1;
	msgFromKernelHdr.msg_control = NULL;
	msgFromKernelHdr.msg_controllen = 0;
	msgFromKernelHdr.msg_flags = 0;

	while (1)
	{
		sleep(WRITE_DB_INTVAL);
		sendmsg(sockd, &msgToKernelHdr, 0);
		recvmsg(sockd, &msgFromKernelHdr, 0);
		p_SessionData = (SessionData *)NLMSG_DATA(nlmsgFromKernelHdr);
		WriteDB(conn, p_SessionData);
	}

	return 0;
}
