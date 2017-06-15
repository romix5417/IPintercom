#ifndef __DECODE_H__
#define __DECODE_H__

#include "audio/aud.h"


int snd_decode_start(Player **player, AudSndCard *card, FILE *aud_encode_fp);
void player_decoder_init(Player **player);
void player_decoder_destroy(Player **player);
#endif
