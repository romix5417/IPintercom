#include <lmlog.h>
#include <speex/speex.h>

#define FRAME_SIZE 160
int snd_encode_start(FILE *aud_raw_fp, FILE *aud_encode_fp)
{
	short in[FRAME_SIZE];
	float input[FRAME_SIZE];

	char cbits[200];
	int  nbBytes;

	// record encode state
	void *state;

	SpeexBits bits;
	int i, tmp;

	state = speex_encoder_init(&speex_nb_mode);
	tmp = 8;
	speex_encoder_ctl(state, SPEEX_SET_QUALITY, &tmp);
    speex_bits_init(&bits);

    while(1){
        fread(in, sizeof(short), FRAME_SIZE, aud_raw_fp);
        if (feof(aud_raw_fp))
            break;
        for (i=0;i<FRAME_SIZE;i++)
            input[i]=in[i];
        speex_bits_reset(&bits);
        speex_encode(state, input, &bits);
        nbBytes = speex_bits_write(&bits, cbits, 200);
        fwrite(&nbBytes, sizeof(int), 1, aud_encode_fp);
        fwrite(cbits, 1, nbBytes, aud_encode_fp);
    }

    speex_encoder_destroy(state);
    speex_bits_destroy(&bits);
    fclose(aud_raw_fp);
    return 0;
}
