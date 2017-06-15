#include <speex/speex.h>
#include <stdio.h>

#include "log/lmlog.h"
#include "audio/encode.h"

#define FRAME_SIZE 160

void recorder_encoder_init(Recorder **recorder)
{
    Encoder *encoder = (*recorder)->encoder;
    encoder->quality = 2;
    encoder->frame_size = FRAME_SIZE;
    //encoder->complexity = 1;
    //encoder->vbr = 1;

    LMLOG(LINF, "%s: Init recorder encoder!, the encoder address is %x.", __FUNCTION__, encoder);

    // record encode state
    encoder->state = speex_encoder_init(&speex_nb_mode);
	speex_encoder_ctl(encoder->state, SPEEX_SET_QUALITY, &(encoder->quality));
    //speex_encoder_ctl(encoder->state, SPEEX_SET_COMPLEXITY, &(encoder->complexity));
    //speex_encoder_ctl(encoder->state,SPEEX_SET_VBR, &(encoder->vbr));
    speex_bits_init(&(encoder->bits));

    LMLOG(LINF, "%s: Init recorder encoder success!the encoder bits is 0x%x.", __FUNCTION__, encoder->bits);

    return;
}

void recorder_encoder_destroy(Recorder **recorder)
{
    LMLOG(LINF, "%s:The &recorder address is 0x%x.",__FUNCTION__, recorder);
    LMLOG(LINF, "%s: Recorder address 0x%x, the encoder address 0x%x!", __FUNCTION__, *recorder, (*recorder)->encoder);
    Encoder *encoder = (*recorder)->encoder;

    LMLOG(LINF, "%s: Destroy recorder encoder!", __FUNCTION__);
    speex_encoder_destroy(encoder->state);
    speex_bits_destroy(&(encoder->bits));

    return;
}

int snd_encode_start(Recorder **recorder, short *raw_data, int size, FILE *aud_encode_fp)
{
    Encoder *encoder = (*recorder)->encoder;
	//short in[FRAME_SIZE];
	float input[FRAME_SIZE];
    int j = 0;

	char cbits[200];
	int  nbBytes;

	// record encode state
	void *state;

	//SpeexBits bits;
	int i;

	//tmp = 8;
	//speex_encoder_ctl(state, SPEEX_SET_QUALITY, &tmp);
    //speex_bits_init(&bits);

    //LMLOG(LINF, "%s: Excute encode start..., the encoder is 0x%x, state is 0x%x, bits is 0x%x,the quality is 0x%x,the frame_size is 0x%x.",
    //             __FUNCTION__, encoder, &(encoder->state), &(encoder->bits), encoder->quality, encoder->frame_size);

    for(j = 0; j < (size/FRAME_SIZE); j++){
        for (i = (0 + j*(size/2)); i<(size/2) + j*(size/2); i++)
            input[i-j*(size/2)]=raw_data[i];

        speex_bits_reset(&(encoder->bits));
        speex_encode(encoder->state, input, &(encoder->bits));
        nbBytes = speex_bits_write(&(encoder->bits), cbits, 200);
        fwrite(&nbBytes, sizeof(int), 1, aud_encode_fp);
        fwrite(cbits, 1, nbBytes, aud_encode_fp);
    }

    //speex_encoder_destroy(state);
    //speex_bits_destroy(&bits);
    return 0;
}
