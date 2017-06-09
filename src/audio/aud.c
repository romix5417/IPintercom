#include <stdio.h>

#include "log/lmlog.h"
#include "audio/aud.h"

#define  __HISA_ENABLE__

AudSndCard *AudCard = NULL;

Recorder *recorder = NULL;

#ifdef __HISA_ENABLE__
extern SndCardDesc hisa_card_desc;
#endif

SndCardDesc *snd_card_descs[] = {
#ifdef __HISA_ENABLE__
    &hisa_card_desc,
#endif
};

void snd_start_play(FILE *aud_raw_fp)
{
    return;
}

void snd_record_stop(void)
{
    return;
}

long get_time(char *cur_time)
{
	long sec = 0;
	time_t now ;
	struct tm *tm_now ;
	sec = time(&now) ;
	tm_now = localtime(&now) ;
	//printf("now datetime: %d-%d-%d %d:%d:%d\n",
	//		tm_now->tm_year+1900, tm_now->tm_mon+1, tm_now->tm_mday, tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec) ;

	sprintf(cur_time, "%d-%d-%d_%d_%d_%d",
						tm_now->tm_year+1900,
						tm_now->tm_mon+1,
						tm_now->tm_mday,
						tm_now->tm_hour,
						tm_now->tm_min,
						tm_now->tm_sec);

	return sec;
}

void snd_record_start(void)
{
    AudSndCard *card;
    Reader *reader;
    char filename[32] = {0};
    char cur_time[16]={0};

    card = recorder->sndCard;
    reader = card->reader;

    if(NULL == recorder->aud_raw_fp){
        get_time(cur_time);
        sprintf(filename,"%02d%s", DEV_NUM,cur_time);
        LMLOG(LINF, "%s: The auido filename is %s.", __FUNCTION__, filename);
        recorder->aud_raw_fp = fopen(filename,"w+");
    }

    reader->snd_read(card, recorder->aud_raw_fp,recorder->sample);
}

void recorder_setup(void)
{
    if(recorder == NULL){
        recorder = (Recorder *)malloc(sizeof(Recorder));
        recorder->sndCard = AudCard;
        recorder->thread_id = NULL;
        recorder->aud_raw_fp = NULL;
        recorder->aud_encode_fp = NULL;
        sample = AUDIO_SAMPLE_RATE_8000;
    }

    return;
}

int snd_play_start(FILE *aud_raw_fp )
{
    AudSndCard *card;
    Writer *writer;

    card = recorder->sndCard;
    writer = card->writer;

    writer->snd_write(card, aud_raw_fp, recorder->sample);
}

int card_detect(SndCardDesc *desc)
{
    if(desc->detect != NULL){
        LMLOG(LINF, "%s: Runing snd card detect!", __FUNCTION__);
        (desc->detect)(&AudCard);
    }

    if(AudCard != NULL){
        LMLOG(LINF, "%s: Detect snd card success!", __FUNCTION__);

        return GOOD;
    }else{
        LMLOG(LINF, "%s: Detect snd card failed!", __FUNCTION__);

        return ERROR;
    }
}

void snd_card_register(SndCardDesc *desc)
{
    int ret = 0;

    ret = card_detect(desc);
    if(ret < 0){
        return;
    }

    (desc->init)(AudCard);
    return;
}

int aud_dev_setup()
{
    int i;

    //  snd card register
    for(i=0; snd_card_descs[i] != NULL; i++){
        snd_card_register(snd_card_descs[i]);
        if(AudCard != NULL)
            return GOOD;
    }

    LMLOG(LERR,"%s:sound card register failed!\r\n", __FUNCTION__);

    return ERROR;
}
