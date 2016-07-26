/*
 * Name:	ParseFeature.c
 *
 * Author:	Feng Wei
 *
 * Date:	2015-02-12
 *
 */

#include <stdio.h>
#include "ConfigFilter.h"

#define APPID_LEN			32
#define LINE_BUF_LEN		128

int readLine(FILE *fp, char *buf)
{
	char c;
	int  i = 0;

	while ( (c = getc(fp)) != EOF && c != '\n')
	{
		if (c != '\r' && c != '\t')
		{
			buf[i++] = c;
		}
	}
	buf[i] = '\0';

	if (c == EOF)
		return EOF;
	else
		return i;
}

int appStart(char *buf)
{
	int i = 0;

	while (buf[i++] == ' ' || buf[i] == '\t');

	if (strncmp(buf+i, "app", 3) == 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int getAppID(char *buf, int len)
{
	int i, j, k;
	char appID[APPID_LEN];
	
	for (i = 0; i+2 < len; i++)
	{
		if (buf[i] == 'i' && buf[i+1] == 'd' && buf[i+2] == '=')
		{
			j = i + 4;
			k = 0;
			while (buf[j] != '\"')
			{
				appID[k++] = buf[j++];
			}
			appID[k] = '\0';
			return atoi(appID);
		}
	}

	return -1;
}

int featureStart(char *buf)
{
	int i = 0;

	while (buf[i++] == ' ' || buf[i] == '\t');

	if (strncmp(buf+i, "feature", 7) == 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

void getFeature(FILE *fp, FeatureConfig *p_Feature)
{
	char buf[LINE_BUF_LEN];
	int  i, j;

	readLine(fp, buf);
	i = 0;
	while (buf[i++] == ' ' || buf[i] == '\t');
	while (buf[i++] != '>');
	if (strncmp(buf+i, "tcp", 3) == 0)
	{
		p_Feature->proto = TCP_PROTO;
	}
	else if (strncmp(buf+i, "udp", 3) == 0)
	{
		p_Feature->proto = UDP_PROTO;
	}

	readLine(fp, buf);
	i = 0;
	while (buf[i++] == ' ' || buf[i] == '\t');
	while (buf[i++] != '>');
	for (j = 0; j < SIG_LEN; j++)
	{
		p_Feature->sigs[j] = strtol(buf+i, NULL, 16);
		i += 3;
	}

	readLine(fp, buf);
	i = 0;
	while (buf[i++] == ' ' || buf[i] == '\t');
	while (buf[i++] != '>');
	for (j = 0; j < SIG_LEN; j++)
	{
		p_Feature->mask[j] = strtol(buf+i, NULL, 16);
		i += 3;
	}

	readLine(fp, buf);
}
