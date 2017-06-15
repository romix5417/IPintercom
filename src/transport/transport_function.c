#include <stdio.h>
#include <stdlib.h>

#include "transport/transport_function.h"
#include "log/lmlog.h"
#include "defs.h"


int snd_ftp_put(char *name)
{
    char buf[128] = {0};

    sprintf(buf, "ftpput -u luomin -p 123456 192.168.10.101 /srv/ftp/upload/%s /mnt/mmc1/%s", name, name);
    LMLOG(LINF, "%s: The ftp put Cmd is '%s'.", __FUNCTION__, buf);

    system(buf);

    return GOOD;
}

int snd_ftp_get(char *name)
{
    char buf[128] = {0};

    sprintf(buf, "ftpget -u luomin -p 123456 192.168.10.101 /mnt/mmc1/%s /srv/ftp/download/%s", name, name);
    LMLOG(LINF, "%s: The ftp get Cmd is '%s'.", __FUNCTION__, buf);

    system(buf);

    return GOOD;
}
