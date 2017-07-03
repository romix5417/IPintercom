#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int get_status()
{
    FILE *fp = NULL;
    char buffer[] = {"linphonecsh status hook | awk \'{print $1}\'"};
    char status[16] = {0};

    fp = popen(buffer,"r");
    if(NULL != fgets(status,13,fp))
    {
        printf("get the status of linphone : %s, the size:%d\r\n", status, sizeof(status));

        pclose(fp);
        if(0 == memcmp(status,"hook=on-hook",12)){
            return 0;
        }

        if(0 == memcmp(status,"hook=ringing",12)){
            return 1;
        }

        if(0 == memcmp(status,"Call",4)){
            return 2;
        }
        else{
            return -1;
        }
    }

    pclose(fp);
    return -1;
}


int main()
{
    int ret;

    ret = get_status();
    printf("ret = %d\r\n", ret);
}
