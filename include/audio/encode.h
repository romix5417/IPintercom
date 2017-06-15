#ifndef __ENCODE_H__
#define __ENCODE_H__

#include "audio/aud.h"

int snd_encode_start(Recorder **recorder, short *raw_data, int size, FILE *aud_encode_fp);
void recorder_encoder_init(Recorder **recorder);
void recorder_encoder_destroy(Recorder **recorder);
#endif
