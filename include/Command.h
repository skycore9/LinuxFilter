/*
 * Name:	Command.h
 *
 * Author:	Feng Wei
 *
 * Date:	2015-02-13
 *
 * Description:	some commands
 */

#ifndef COMMAND_H
#define COMMAND_H

#define CMDNAME_LEN				16
#define PARAMETER_LEN			128

typedef struct Command
{
	char			mainCmdName[CMDNAME_LEN];
	char			subCmdName[CMDNAME_LEN];
	unsigned char	param[PARAMETER_LEN];
}Command;

typedef struct UserConfig
{
	unsigned int	ip;
	int				level;
}UserConfig;

typedef struct FeatureConfig
{
#ifndef SIG_LEN
#define SIG_LEN		14
#endif
	int				appID;
	unsigned char	sigs[SIG_LEN];
	unsigned char	mask[SIG_LEN];
	unsigned char	proto;
}FeatureConfig;

typedef struct PolicyConfig
{
	int level;
	int appConfigID;
}PolicyConfig;

#endif
