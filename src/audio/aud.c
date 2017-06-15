#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#include "log/lmlog.h"
#include "audio/aud.h"
#include "message/message.h"
#include "audio/decode.h"

#define  __HISA_ENABLE__

AudSndCard *AudCard = NULL;

Recorder *recorder = NULL;

Player *player = NULL;

#define AUDIO_SAMPLE_RATE_8000 8000

extern bool Record_start;

extern int DEV_NUM;

#ifdef __HISA_ENABLE__
extern SndCardDesc InterCom_hisa_card_desc;
#endif

SndCardDesc *snd_card_descs[] = {
#ifdef __HISA_ENABLE__
    &InterCom_hisa_card_desc,
#endif
};

void snd_play_start(AudSndCard *card)
{
    VoiceMessage *vmsg = NULL;
    char buf[32];
    int fd = 0;

    LMLOG(LINF, "%s: Excute sound play start.", __FUNCTION__);

    player_decoder_init(&player);
    (card->desc->write_preprocess)();

    while(fifo_dequeue(player->msg_fifo,&vmsg) == FIFO_OK){
        sprintf(buf, "/mnt/mmc1/%s", vmsg->filename);

        LMLOG(LINF, "%s: The vmsg filename %s, vmsg address:%x, vmsg filename address:%x.", __FUNCTION__, vmsg->filename, vmsg, vmsg->filename);
        memcpy(player->play_aud_file, vmsg->filename, sizeof(player->play_aud_file));

        LMLOG(LINF, "%s: The play filename %s", __FUNCTION__, buf);
        player->play_aud_fp = fopen(buf,"r");
        if(player->play_aud_fp != NULL){
            snd_decode_start(&player, player->sndCard, player->play_aud_fp);
            fclose(player->play_aud_fp);
            player->play_aud_fp = NULL;
        }else{
            LMLOG(LERR, "%s: Open the aud file failed.", __FUNCTION__);
        }

        memset(buf,0,32);
        if(vmsg!=NULL){
            free(vmsg);
            vmsg = NULL;
        }
    }

    (card->desc->write_postprocess)();
    player_decoder_destroy(&player);

    player->play_flag = STOPPING;
    //player_destroy();

    return;
}

void snd_record_stop(void)
{
    Record_start = false;
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

	sprintf(cur_time, "%4d%02d%02d%02d%02d%02d",
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
    char cur_time[16]={0};
    char buf[64]={0};

    card = recorder->sndCard;
    reader = card->reader;
    recorder->state = RUNNING;

    if(NULL == recorder->aud_raw_fp){
        get_time(cur_time);
        sprintf(recorder->aud_encode_file,"%02d%s", DEV_NUM,cur_time);
        LMLOG(LINF, "%s: The auido filename is %s.", __FUNCTION__, recorder->aud_encode_file);
        sprintf(buf, "/mnt/mmc1/%s",recorder->aud_encode_file);
        recorder->aud_raw_fp = fopen(buf,"w+");
    }

    // init encoder
    recorder_encoder_init(&recorder);
    // prepare for read hisi aud data
    (card->desc->read_preprocess)();

    Record_start = true;
    if(reader != NULL)
        reader->snd_read(recorder, card, recorder->aud_raw_fp,recorder->sample);
    else
        LMLOG(LERR, "%s: The reader is NULL.", __FUNCTION__);

    (card->desc->read_postprocess)();

    recorder_encoder_destroy(&recorder);

    recorder->state = STOPPING;
    return;
}

void recorder_setup(void)
{
    if(recorder == NULL){
        recorder = (Recorder *)malloc(sizeof(Recorder));
        recorder->sndCard = AudCard;
        recorder->aud_raw_fp = NULL;
        memset(recorder->aud_encode_file, 0, sizeof(recorder->aud_encode_file));
        recorder->sample = AUDIO_SAMPLE_RATE_8000;
        recorder->encoder = (Encoder *)malloc(sizeof(Encoder));
        LMLOG(LINF, "%s: The recorder encoder is %x.", __FUNCTION__, recorder->encoder);
    }

    return;
}

void recorder_destroy(void)
{
    LMLOG(LINF, "%s: Destroy the recorder.", __FUNCTION__);
    if(recorder != NULL){
        if(recorder->encoder != NULL){
            free(recorder->encoder);
            recorder->encoder = NULL;
        }

        free(recorder);
        recorder = NULL;
    }

    return;
}

void player_setup(void)
{
    LMLOG(LINF, "%s: The player setup.", __FUNCTION__);
    if(player == NULL){
        player = (Player *)malloc(sizeof(Player));
        player->sndCard = AudCard;
        player->play_aud_fp = NULL;
        memset(player->play_aud_file, 0, sizeof(player->play_aud_file));
        player->sample = AUDIO_SAMPLE_RATE_8000;
        player->msg_fifo = fifo_new(sizeof(VoiceMessage *),16,0);
        player->play_flag = STOPPING;
        player->decoder = (Decoder *)malloc(sizeof(Decoder));
    }

    return;
}

void player_destroy(void)
{
    LMLOG(LINF, "%s: Destroy the player.", __FUNCTION__);
    if(player !=NULL){
        if(player->msg_fifo != NULL){
            fifo_destroy(player->msg_fifo);
            player->msg_fifo = NULL;
        }
        if(player->decoder != NULL){
            free(player->decoder);
            player->decoder = NULL;
        }

        free(player);
        player = NULL;
    }

    return;
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

    (desc->init)(desc,AudCard);
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
