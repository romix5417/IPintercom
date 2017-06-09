#include <audio/aud.h>
#include <stdio.h>
#include <stdlib.h>

#include "log/lmlog.h"

static int snd_read(struct AudSndCard *card, FILE *fp, int sample)
{

}

static int snd_write(struct AudSndCard *card, FILE *fp, int sample)
{

}

static void hisa_card_detect(AudSndCard **card)
{
    *card = (AudSndCard *)malloc(sizeof(AudSndCard));
    LMLOG(LINF, "%s: Register snd card success!", __FUNCTION__);

    return;
}

static void hisa_card_init(AudSndCard *card)
{
    LMLOG(LINF, "%s: Init snd card success!", __FUNCTION__);

    return;
}

static void hisa_card_create_reader(AudSndCard *card)
 {
    if(NULL == card->reader){
        card->reader = (Reader *)malloc(sizeof(Reader));
        card->reader->snd_read = snd_read;
    }else{
        LMLOG(LINF, "%s: The Reader haved Register!", __FUNCTION__);
        return;
    }

    LMLOG(LINF, "%s: Create snd card Reader success!", __FUNCTION__);

    return;
}

static void hisa_card_create_writer(AudSndCard *card)
{
    if(NULL == card->writer){
        card->writer = (Writer *)malloc(sizeof(Writer));
        card->writer->snd_write = snd_write;
    }else{
        LMLOG(LINF, "%s: The Write haved Register!", __FUNCTION__);
        return;
    }

    LMLOG(LINF, "%s: Create snd card Writer success!", __FUNCTION__);

    return;
}

static void hisa_card_unload(AudSndCard *card)
{
    LMLOG(LINF, "%s: Unload snd card success!", __FUNCTION__);

    return;
}

SndCardDesc hisa_card_desc = {
    .detect = hisa_card_detect,
    .init   = hisa_card_init,
    .create_reader = hisa_card_create_reader,
    .create_writer = hisa_card_create_writer,
    .unload = hisa_card_unload
};
