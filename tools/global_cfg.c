#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>


typedef struct Cfg_Opt{
    int  dev_num;
    int  resolution;
    int  frame;
}CFG_OPT;

// cinfigure struct
struct configItem
{
	char key[20];
	char value[50];
}  configList[] =	{
	{"dev_num", 0},
    {"resolution", 0},
	{"frame", 0},
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
		printf("%s: Open %s file failed.\n", __FUNCTION__, configFilePath);
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

int cfg_read(char *cfg_name, CFG_OPT *global_cfg)
{
	int i;
    int ret;

    printf("%s: Read the cfg file!\n", __FUNCTION__);

	ret = config(cfg_name, configList, sizeof(configList)/sizeof(struct configItem));
    if(ret < 0){
        printf("%s: Read the cfg file failed!\n", __FUNCTION__);
        return ret;
    }


	for(i=0; i<sizeof(configList)/sizeof(struct configItem); i++)
	{

        //printf("%s: %s = %s\r\n", __FUNCTION__, configList[i].key, configList[i].value);
        if( 0 == memcmp(configList[i].key, "dev_num", 7)){
            global_cfg->dev_num = atoi(configList[i].value);
            printf("%s: Global The %s = %d.\n", __FUNCTION__, configList[i].key, global_cfg->dev_num);
        }

        if( 0 == memcmp(configList[i].key, "resolution", 10)){
            global_cfg->resolution = atoi(configList[i].value);
            printf("%s: Global The %s = %d.\n", __FUNCTION__, configList[i].key, global_cfg->resolution);
        }

        if( 0 == memcmp(configList[i].key, "frame", 5)){
            global_cfg->frame = atoi(configList[i].value);
            printf("%s: Global The %s = %d.\n", __FUNCTION__, configList[i].key, global_cfg->frame);
        }
	}

	return 0;
}


int get_pid(char *buf)
{
    FILE *fp = NULL;
    char buffer[100]={0};
    char cpid[10]={0};

    sprintf(buffer,"ps -a | grep \'%s\' | awk \'{print $1}\'",buf);
    fp = popen(buffer,"r");
    if(NULL != fgets(cpid,10,fp))
    {
             printf("get the stop program pid: %s",cpid);

             pclose(fp);
            return atoi(cpid);
        }

    pclose(fp);

    return 0;
}

int main()
{
    char buf[32]={0};
    char buf_cmd[32]={0};
    char h265_cmd[32] = {"ETWS_RtspServer_H265 -s 1 &"};
    char h264_cmd[32] = {"ETWS_RtspServer_H264 -s 1"};
    int pid = 0;

    CFG_OPT  global_cfg = {
        .dev_num = 1,
        .resolution = -1,
        .frame = 25
    };

    cfg_read("/etc/config/rtspServer.cfg", &global_cfg);

    switch(global_cfg.resolution){
        case 0:
            memcpy(buf, h265_cmd, sizeof(h265_cmd));

            pid = get_pid("linphonec");
            if(pid != 0){
                    kill(pid,SIGKILL);
            }
            pid = 0;

            pid = get_pid("ETWS_RtspServer_H264");
            if(pid != 0){
                    kill(pid,SIGKILL);
            }
            pid = 0;

            pid = get_pid("ETWS_RtspServer_H265");
            if(pid != 0){
                    kill(pid,SIGKILL);
            }
            pid = 0;

            system("linphonecsh init -c /etc/config/linphonerc");
            printf("buf is :%s\r\n", buf);
            usleep(1500000);
            system(buf);

            sprintf(buf_cmd,"/etc/config/1080p_cfg.sh %d", global_cfg.frame);
            printf("buf cmd :%s\r\n", buf_cmd);
            usleep(1000000);
            system(buf_cmd);

            break;
        case 1:
        case 2:
        case 3:
            sprintf(buf, "%s -r %d &", h264_cmd, global_cfg.resolution-1);

            pid = get_pid("linphonec");
            if(pid != 0){
                    kill(pid,SIGKILL);
            }
            pid = 0;

            pid = get_pid("ETWS_RtspServer_H264");
            if(pid != 0){
                    kill(pid,SIGKILL);
            }
            pid = 0;

            pid = get_pid("ETWS_RtspServer_H265");
            if(pid != 0){
                    kill(pid,SIGKILL);
            }
            pid = 0;
            system("linphonecsh init -c /etc/config/linphonerc");
            printf("buf is :%s\r\n", buf);
            usleep(1500000);
            system(buf);

            usleep(1000000);
            switch(global_cfg.resolution){
                case 1:
                    sprintf(buf_cmd,"/etc/config/720p_cfg.sh %d", global_cfg.frame);
                    system(buf_cmd);
                    printf("buf cmd :%s\r\n", buf_cmd);
                    break;
                case 2:
                    sprintf(buf_cmd,"/etc/config/480p_cfg.sh %d", global_cfg.frame);
                    system(buf_cmd);
                    printf("buf cmd :%s\r\n", buf_cmd);
                    break;
                case 3:
                    sprintf(buf_cmd,"/etc/config/240p_cfg.sh %d", global_cfg.frame);
                    system(buf_cmd);
                    printf("buf cmd :%s\r\n", buf_cmd);
                    break;
                default:
                    break;
            }

            break;

        default:
            break;
    }

}
