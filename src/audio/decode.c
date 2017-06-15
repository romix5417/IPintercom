#include <stdio.h>
#include <speex/speex.h>

#include "defs.h"
#include "audio/decode.h"
#include "log/lmlog.h"


void player_decoder_init(Player **player)
{
    Decoder *decoder = (*player)->decoder;

    decoder->enh = 0;
    decoder->state = speex_decoder_init(&speex_nb_mode);

    LMLOG(LINF, "%s: Init decorder encoder!, the decoder address is %x.", __FUNCTION__, decoder);
    speex_decoder_ctl(decoder->state, SPEEX_SET_ENH, &(decoder->enh));
    speex_bits_init(&(decoder->bits));

    LMLOG(LINF, "%s: Init recorder encoder success!", __FUNCTION__);

    return;
}

void player_decoder_destroy(Player **player)
{
    Decoder *decoder = (*player)->decoder;

    /*Destroy the decoder state*/
    speex_decoder_destroy(decoder->state);
    /*Destroy the bit-stream truct*/
    speex_bits_destroy(&(decoder->bits));

    return;
}

/*The frame size in hardcoded for this sample code but it doesn't have to be*/
#define FRAME_SIZE 160
int snd_decode_start(Player **player, AudSndCard *card, FILE *aud_encode_fp)
{
    Decoder *decoder = (*player)->decoder;
	char *outFile;
    //FILE *fout = aud_encode_fp;
	/*Holds the audio that will be written to file (16 bits per sample)*/
    short out[FRAME_SIZE];
    /*Speex handle samples as float, so we need an array of floats*/
    float output[FRAME_SIZE];
    char cbits[200];
    int nbBytes;
    /*Holds the state of the decoder*/
    //void *state;
    /*Holds bits so they can be read and written to by the Speex routines*/
    //SpeexBits bits;
    //int i, tmp;
    int i;

    /*Create a new decoder state in narrowband mode*/
    //state = speex_decoder_init(&speex_nb_mode);

    /*Set the perceptual enhancement on*/
    //tmp=1;
    //speex_decoder_ctl(state, SPEEX_SET_ENH, &tmp);

    /*Initialization of the structure that holds the bits*/
    //speex_bits_init(&bits);
    while (1){
        /*Read the size encoded by sampleenc, this part will likely be 
        different in your application*/
        //LMLOG(LINF, "%s: Decode the aud file, fread the size. the aud file fp address is 0x%x.", __FUNCTION__, aud_encode_fp);

        fread(&nbBytes, sizeof(int), 1, aud_encode_fp);
        //fprintf (stderr, "nbBytes: %d\n", nbBytes);
        if (feof(aud_encode_fp))
            break;

        /*Read the "packet" encoded by sampleenc*/
        //LMLOG(LINF, "%s: Decode the aud file, fread the data.the aud file fp address is 0x%x.", __FUNCTION__, aud_encode_fp);
        fread(cbits, 1, nbBytes, aud_encode_fp);
        /*Copy the data into the bit-stream struct*/
        speex_bits_read_from(&(decoder->bits), cbits, nbBytes);

        /*Decode the data*/
        speex_decode(decoder->state, &(decoder->bits), output);

        /*Copy from float to short (16 bits) for output*/
        for (i=0;i<FRAME_SIZE;i++)
         out[i]=output[i];

        /*Write the decoded audio to file*/
        //fwrite(out, sizeof(short), FRAME_SIZE, fout);
        (card->writer->snd_write)(card, (char*)out, FRAME_SIZE);

    }

    /*Destroy the decoder state*/
    //speex_decoder_destroy(state);
    /*Destroy the bit-stream truct*/
    //speex_bits_destroy(&bits);
    //fclose(fout);

    LMLOG(LINF, "%s: Aud file decode success.", __FUNCTION__);

    return GOOD;
}
