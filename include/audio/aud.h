#ifndef __AUD_H_
#define __AUD_H_

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#include <speex/speex.h>

#include "fifo/fifo.h"


typedef struct speexDecoder{
    void *state;
    SpeexBits bits;
    int enh;
    int frame_size;
}Decoder;

typedef struct speexEncoder{
    void *state;
    SpeexBits bits;
    int complexity;
    int quality;
    int frame_size;
    int vbr;
}Encoder;

struct AudSndCard;
struct SndCardDesc;
struct Recorder;
struct Player;

typedef struct Reader{
    int (*snd_read)(struct Recorder *recorder, struct AudSndCard *card, FILE *fp, int sample);
}Reader;

typedef struct Writer{
    int (*snd_write)(struct AudSndCard *card, char *pstData, int samples);
}Writer;

typedef struct Recorder{
    struct AudSndCard *sndCard;
    pthread_t  thread_id;
    char       record_aud_file[32];
    FILE       *aud_raw_fp;
    int        sample;
    Encoder    *encoder;
    int        state;
}Recorder;

typedef struct Player{
    struct AudSndCard *sndCard;
    char   play_aud_file[16];
    FILE   *play_aud_fp;
    int    sample;
    fifo_t *msg_fifo;
    int    play_flag;
    Decoder *decoder;
}Player;

typedef struct AudSndCard{
    char name[32];
    int  cardId;
    int  chn;
    int  devId;
    void *data;
    Reader *reader;
    Writer *writer;
    struct SndCardDesc *desc;
}AudSndCard;

typedef struct SndCardDesc{
    char driver_type[32];
    void (*detect)(AudSndCard **card);
    void (*init)(struct SndCardDesc *desc, AudSndCard *card);
    void (*read_preprocess)(void);
    void (*read_postprocess)(void);
    void (*write_preprocess)(void);
    void (*write_postprocess)(void);
    void (*create_reader)(AudSndCard *card);
    void (*create_writer)(AudSndCard *card);
    void (*unload)(AudSndCard *card);
}SndCardDesc;


void snd_record_start(void);
void snd_record_stop(void);
void snd_play_start(AudSndCard *card);
void recorder_setup(void);
void player_destroy(void);
void recorder_destroy(void);
int aud_dev_setup();

#endif
