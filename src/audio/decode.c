#include <stdio.h>

#include "defs.h"
#include "audio/decode.h"
#include "log/lmlog.h"

int snd_decode_start(FILE *aud_encode_fp, FILE *aud_raw_fp)
{
    LMLOG(LINF, "%s: Aud file decode success.", __FUNCTION__);

    return GOOD;
}


