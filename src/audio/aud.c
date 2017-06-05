#include <stdio.h>
#include <lmlog.h>
#include <audio/aud.h>

AudSndCard *AudCard = NULL;

Recorder *recorder = NULL;

#ifdef __HISA_ENABLE__
extern hisa_card_desc;
#endif

SndCardDesc *snd_card_descs[] = {
#ifdef __HISA_ENABLE__
    &hisa_card_desc,
#endif
};

void snd_record_stop(void)
{
    return;
}

void snd_record_start(void)
{
    AudSndCard *card;
    Reader *reader;

    card = recorder->sndCard;
    reader = card->reader;

    reader->snd_read(card, recorder->aud_raw_fp,recorder->sample);
}

int recorder_setup(void)
{
    return GOOD;
}

int snd_play_start(FILE *aud_raw_fp, )
{
    AudSndCard *card;
    Writer *writer;

    card = recorder->sndCard;
    writer = card->writer;

    writer->snd_write(card, aud_raw_fp, recorder->sample);
}

int card_detect(SndCardDesc *desc)
{
    if(desc->detect != NULL)
        (desc->detect)(AudCard);

    if(AudCard != NULL)
        return SUCCESS;
    else
        return ERROR;
}

void snd_card_register(SndCardDesc *desc)
{
    int ret = 0;

    ret = card_detect(desc);
    if(ret < 0){
        return NULL;
    }

    (desc->init)(AudCard);
    return AudCard;
}

int aud_dev_setup()
{
    int i;

    //  snd card register
    for(i=0; snd_card_descs[i] != NULL; i++){
        snd_card_register(snd_card_descs[i]);
        if(AudCard != NULL)
            return SUCCESS;
    }

    LMLOG(LOG_ERR,"%s:sound card register failed!\r\n", __FUNCTION__);

    return ERROR;
}
