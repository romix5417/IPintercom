#include <stdio.h>
#include <stdlib.h>

#include "transport/transport_function.h"
#include "log/lmlog.h"
#include "cfg/cfg.h"
#include "defs.h"

extern CFG_OPT global_cfg;

int snd_ftp_put(char *name)
{
    char buf[128] = {0};

    sprintf(buf, "ftpput -u etws -p etws -P %d %s %s /mnt/mmc1/%s", global_cfg.ftp_port, global_cfg.host_ip, name, name);
    LMLOG(LINF, "%s: The ftp put Cmd is '%s'.", __FUNCTION__, buf);

    system(buf);

    return GOOD;
}

int snd_ftp_get(char *name)
{
    char buf[128] = {0};

    sprintf(buf, "ftpget -u etws -p etws -P %d %s /mnt/mmc1/%s %s", global_cfg.ftp_port, global_cfg.host_ip, name, name);
    LMLOG(LINF, "%s: The ftp get Cmd is '%s'.", __FUNCTION__, buf);

    system(buf);

    return GOOD;
}
