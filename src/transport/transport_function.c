#include <stdio.h>
#include <stdlib.h>

#include "transport/transport_function.h"
#include "defs.h"

int snd_ftp_put(char *name)
{
    return GOOD;
}

int snd_ftp_get(char *name)
{
    char buf[128] = {0};

    sprintf(buf, "ftpget -u luomin -p 123456 192.168.10.101 %s /srv/ftp/download/%s.wav", name, name);

    system(buf);

    return GOOD;
}
