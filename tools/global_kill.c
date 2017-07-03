#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>

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
    int pid = 0;

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

    return 0;
}
