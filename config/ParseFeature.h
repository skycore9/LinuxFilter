/*
 * Name:	ParseFeature.h
 *
 * Author:	Feng Wei
 *
 * Date:	2015-02-13
 */

#ifndef PARSE_FEATURE_H
#define PARSE_FEATURE_H

#include <stdio.h>
#include "ConfigFilter.h"

#define LINE_BUF_LEN		128

int readLine(FILE *fp, char *buf);

int appStart(char *buf);

int getAppID(char *buf, int len);

int featureStart(char *buf);

int getFeature(FILE *fp, FeatureConfig *p_Feature);
#endif
