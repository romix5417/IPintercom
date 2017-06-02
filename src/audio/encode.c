#include <lmlog.h>
#include <speex/speex.h>


int snd_encode_start(FILE *aud_raw_fp, FILE *aud_encode_fd)
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
        if (feof(fin))
            break;
        for (i=0;i<FRAME_SIZE;i++)
            input[i]=in[i];
        speex_bits_reset(&bits);
        speex_encode(state, input, &bits);
        nbBytes = speex_bits_write(&bits, cbits, 200);
        fwrite(&nbBytes, sizeof(int), 1, stdout);
        fwrite(cbits, 1, nbBytes, stdout);
    }

}


#define FRAME_SIZE 160
int main(int argc, char **argv)
{
    char *inFile;
    FILE *fin;
    short in[FRAME_SIZE];
    float input[FRAME_SIZE];
    char cbits[200];
    int nbBytes;
    /*保存编码的状态*/
    void *state;
    /*保存字节因此他们可以被speex常规读写*/
 
    SpeexBits bits;
    int i, tmp;
    //新建一个新的编码状态在窄宽(narrowband)模式下
    state = speex_encoder_init(&speex_nb_mode);
    //设置质量为8(15kbps)
    tmp=8;
    speex_encoder_ctl(state, SPEEX_SET_QUALITY, &tmp);
    inFile = argv[1];
    fin = fopen(inFile, "r");
    //初始化结构使他们保存数据
    speex_bits_init(&bits);
    while (1)
    {
        //读入一帧16bits的声音
        fread(in, sizeof(short), FRAME_SIZE, fin);
        if (feof(fin))
            break;
        //把16bits的值转化为float,以便speex库可以在上面工作
        for (i=0;i<FRAME_SIZE;i++)
            input[i]=in[i];
        //清空这个结构体里所有的字节,以便我们可以编码一个新的帧
        speex_bits_reset(&bits);
        //对帧进行编码
        speex_encode(state, input, &bits);
        //把bits拷贝到一个利用写出的char型数组
        nbBytes = speex_bits_write(&bits, cbits, 200);
        //首先写出帧的大小,这是sampledec文件需要的一个值,但是你的应用程序中可能不一样
        fwrite(&nbBytes, sizeof(int), 1, stdout);
        //写出压缩后的数组
        fwrite(cbits, 1, nbBytes, stdout);
    }

    //释放编码器状态量
    speex_encoder_destroy(state);
    //释放bit_packing结构
    speex_bits_destroy(&bits);
    fclose(fin);
    return 0;
}
