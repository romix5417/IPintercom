#ifndef __AUD_H_
#define __AUD_H_

typedef struct Reader{
    int (*snd_read)(AudSndCard *card, FILE *fp, int sample);
}Reader;

typedef struct Writer{
    int (*snd_write)(AudSndCard *card, FILE *fp, int sample);
}Writer;

typedef struct AudSndCard{
    char name[32];
    int  cardId;
    int  chn;
    int  devId;
    void *data;
    Reader *reader;
    Writer *writer;
}AudSndCard;

typedf struct SndCardDesc{
    char driver_type[32];
    void (*detect)(AudSndCard *card);
    void (*init)(AudSndCard *card);
    void (*create_reader)(AudSndCard *card);
    void (*create_writer)(AudSndCard *card);
    void (*unload)(AudSndCard *card);
}SndCardDesc;

typedef struct Recorder{
    AudSndCard *sndCard;
    pthread_t  *thread_id;
    FILE       *aud_raw_fp;
    FILE       *aud_encode_fp;
    int        sample;
}Recorder;

void snd_record_start(void);
void snd_record_stop(void);
void recorder_setup(void);


#endif
