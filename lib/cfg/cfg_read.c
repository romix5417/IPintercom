#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "cfg/cfg.h"
#include "log/lmlog.h"

// cinfigure struct
struct configItem
{
	char key[20];
	char value[50];
}  configList[] =	{
	{"host_ip", 0},
    {"host_num", 0},
	{"dev_num", 0},
	{"debug_level", 0},
	{"dest_port", 0},
    {"local_port", 0},
	{"ftp_port", 0}
};

/*
 * find the config file key and value
 * @param  src	 source [input  value]
 * @param  key	 key    [output value]
 * @param value	 value  [output value]
 */
int strkv(char *src, char *key, char *value)
{
	char *p,*q;
	int len;
	p = strchr(src, '=');	// p find the equal
	q = strchr(src, '\n');	// q find the line feed

	// if have both equal and line feed 
	if (p != NULL && q != NULL)
	{
		*q = '\0';
		strncpy(key, src, p - src);
		strcpy(value, p+1);
		return 1;
	}else
	{
		return 0;
	}
}

/*
 * configure function
 * @param configFilePath configure path
 * @param   configVar
 * @param   configNum
 */
int config(char *configFilePath, struct configItem * configVar, int configNum)
{
	int i;
	FILE *fd;
	char buf[50]={0};
	char key[50]={0};
	char value[50]={0};

	fd = fopen(configFilePath, "r");
    if(fd <= 0){
		LMLOG(LERR, "%s: Open %s file failed.", __FUNCTION__, configFilePath);
        return -1;
    }

	while(fgets(buf, 50, fd))
	{
		if (strkv(buf, key, value))
		{
			for(i = 0; i< configNum; i++)
			{
				if (strcmp(key, configVar[i].key) == 0)
				{
					strcpy(configVar[i].value, value);
				}
			}
			memset(key, 0, strlen(key));
		}
	}

	fclose(fd);

    return 0;
}

int cfg_read(char *cfg_name, CFG_OPT * global_cfg)
{
	int i;
    int ret;

    LMLOG(LINF, "%s: Read the cfg file!", __FUNCTION__);

	ret = config(cfg_name, configList, sizeof(configList)/sizeof(struct configItem));
    if(ret < 0){
        LMLOG(LERR, "%s: Read the cfg file failed!", __FUNCTION__);
        return ret;
    }

	for(i=0; i<sizeof(configList)/sizeof(struct configItem); i++)
	{
		//LMLOG(LINF, "%s: %s = %s", __FUNCTION__, configList[i].key, configList[i].value);

        if( 0 == memcmp(configList[i].key, "host_ip", 7)){
            memcpy(global_cfg->host_ip,configList[i].value, sizeof(global_cfg->host_ip));
            LMLOG(LINF, "%s: Global The %s = %s.", __FUNCTION__, configList[i].key, global_cfg->host_ip);
        }

        if( 0 == memcmp(configList[i].key, "host_num", 7 )){
            global_cfg->host_num = atoi(configList[i].value);
            LMLOG(LINF, "%s: Global The %s = %d.", __FUNCTION__, configList[i].key, global_cfg->host_num);
        }

        if( 0 == memcmp(configList[i].key, "dev_num", 7)){
            global_cfg->dev_num = atoi(configList[i].value);
            LMLOG(LINF, "%s: Global The %s = %d.", __FUNCTION__, configList[i].key, global_cfg->dev_num);
        }

        if( 0 == memcmp(configList[i].key, "debug_level", 11)){
            global_cfg->debug_level = atoi(configList[i].value);
            LMLOG(LINF, "%s: Global The %s = %d.", __FUNCTION__, configList[i].key, global_cfg->debug_level);
        }

        if( 0 == memcmp(configList[i].key, "dest_port", 9)){
            global_cfg->dest_port = atoi(configList[i].value);
            LMLOG(LINF, "%s: Global The %s = %d.", __FUNCTION__, configList[i].key, global_cfg->dest_port);
        }

        if( 0 == memcmp(configList[i].key, "local_port", 10)){
            global_cfg->local_port = atoi(configList[i].value);
            LMLOG(LINF, "%s: Global The %s = %d.", __FUNCTION__, configList[i].key, global_cfg->local_port);
        }

        if( 0 == memcmp(configList[i].key,"ftp_port", 8)){
            global_cfg->ftp_port = atoi(configList[i].value);
            LMLOG(LINF, "%s: Global The %s = %d.", __FUNCTION__, configList[i].key, global_cfg->ftp_port);
        }
	}

	return 0;
}
