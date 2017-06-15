#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "defs.h"
#include "log/lmlog.h"
#include "audio/aud.h"
#include "audio/encode.h"
#include "audio/decode.h"
#include "audio/hisa.h"

#include "mpp/hi_comm_aio.h"
#include "mpp/hi_common.h"
#include "mpp/acodec.h"

bool Record_start = false;

#define SAMPLE_AUDIO_AI_DEV 0
#define SAMPLE_AUDIO_AO_DEV 0

#define ACODEC_FILE	"/dev/acodec"

#define AUDIO_ADPCM_TYPE ADPCM_TYPE_DVI4 /* ADPCM_TYPE_IMA, ADPCM_TYPE_DVI4*/
#define G726_BPS MEDIA_G726_40K          /* MEDIA_G726_16K, MEDIA_G726_24K ... */

static HI_BOOL gs_bAioReSample  = HI_FALSE;
static AUDIO_SAMPLE_RATE_E enInSampleRate  = AUDIO_SAMPLE_RATE_BUTT;
static AUDIO_SAMPLE_RATE_E enOutSampleRate = AUDIO_SAMPLE_RATE_BUTT;
static HI_U32 u32AencPtNumPerFrm = 320;
static PAYLOAD_TYPE_E gs_enPayloadType = PT_LPCM;

AIO_ATTR_S AiDevAttr;
AIO_ATTR_S AoDevAttr;

static int snd_read(Recorder *recorder, struct AudSndCard *card, FILE *fp, int sample)
{
	int size = 0;
	int s32Ret;
    AUDIO_FRAME_S stFrame;
    AEC_FRAME_S   stAecFrm;
    AI_CHN_PARAM_S stAiChnPara;
    int AiDevId = 0;
    int AiChn = 0;

    while(Record_start){
        s32Ret = HI_MPI_AI_GetChnParam(AiDevId, AiChn, &stAiChnPara);
        if (HI_SUCCESS != s32Ret){
            LMLOG(LERR, "%s: Get ai chn param failed\n", __FUNCTION__);
            return BAD;
        }

        stAiChnPara.u32UsrFrmDepth = 30;

        s32Ret = HI_MPI_AI_SetChnParam(AiDevId, AiChn, &stAiChnPara);
        if (HI_SUCCESS != s32Ret){
            LMLOG(LERR, "%s: set ai chn param failed\n", __FUNCTION__);
            return BAD;
        }

        /* get frame from ai chn */
        memset(&stAecFrm, 0, sizeof(AEC_FRAME_S));

        s32Ret = HI_MPI_AI_GetFrame(AiDevId, AiChn, &stFrame, &stAecFrm, -1);
        if (HI_SUCCESS != s32Ret ){
            LMLOG(LERR, "%s: HI_MPI_AI_GetFrame(%d, %d), failed with %#x!", \
                __FUNCTION__, AiDevId, AiChn, s32Ret);
            return BAD;
            //continue;
        }

        size = stFrame.u32Len / (sizeof(short)) ;
        LMLOG(LINF, "%s: The frame size is %d.", __FUNCTION__, size);

        snd_encode_start(&recorder, (short *)stFrame.pVirAddr[0], size, fp);

        s32Ret = HI_MPI_AI_ReleaseFrame(AiDevId, AiChn, &stFrame, &stAecFrm);
        if (HI_SUCCESS != s32Ret ){
            LMLOG(LERR, "%s: HI_MPI_AI_ReleaseFrame(%d, %d), failed with %#x!", \
                   __FUNCTION__, AiDevId, AiChn, s32Ret);
            return BAD;
        }
    }

    return GOOD;
}

static int snd_write(struct AudSndCard *card, char *pstData, int samples)
{
    HI_S32      s32ret;
	//int         outlen;
    AUDIO_DEV   AoDevId = SAMPLE_AUDIO_AO_DEV;
    AO_CHN      AoChn = 0;
    ADEC_CHN    AdChn = 0;
    HI_S32      s32AoChnCnt;
    AIO_ATTR_S stAioAttr;
    HI_S32 s32MilliSec = 10;
    AUDIO_STREAM_S stStream;
	//unsigned char aoData[1520];
    int i = 0;

	//outlen = HisiVoiceAddHisiHeader((short *)pstData,(short *)aoData,512,samples);
    //stStream.u32Len  = outlen;
    //stStream.pStream = aoData;
    stStream.u32Len  = samples * 2;
    stStream.pStream = pstData;
    int j = 0;

    if(i){
        for(j = 0; j < samples * 2; j++)
        LMLOG(LINF, "%s: HI_MPI_ADEC_SendStream:0x%x ", __FUNCTION__, *(pstData+i));
        i--;
    }

	s32ret = HI_MPI_ADEC_SendStream(AdChn, &stStream, HI_TRUE);
    LMLOG(LINF, "%s: HI_MPI_ADEC_SendStream(%d), send stream s32ret is %#x,samples is %d", \
			   __FUNCTION__, AdChn, s32ret, samples);
	if (HI_SUCCESS != s32ret )
	{
		LMLOG(LERR, "%s: HI_MPI_ADEC_SendStream(%d), failed with %#x!", \
			   __FUNCTION__, AdChn, s32ret);
		return s32ret;
	}else
        return samples;
}

VIDEO_NORM_E gs_enNorm = VIDEO_ENCODING_MODE_NTSC;

/******************************************************************************
* function : vb init & MPI system init
******************************************************************************/
HI_S32 HISA_COMM_SYS_Init(VB_CONF_S* pstVbConf)
{
    MPP_SYS_CONF_S stSysConf = {0};
    HI_S32 s32ret = HI_FAILURE;

    HI_MPI_SYS_Exit();
    HI_MPI_VB_Exit();

    if (NULL == pstVbConf)
    {
        SAMPLE_PRT("input parameter is null, it is invaild!\n");
        return HI_FAILURE;
    }

    s32ret = HI_MPI_VB_SetConf(pstVbConf);
    if (HI_SUCCESS != s32ret)
    {
        SAMPLE_PRT("HI_MPI_VB_SetConf failed!\n");
        return HI_FAILURE;
    }

    s32ret = HI_MPI_VB_Init();
    if (HI_SUCCESS != s32ret)
    {
        SAMPLE_PRT("HI_MPI_VB_Init failed!\n");
        return HI_FAILURE;
    }

    stSysConf.u32AlignWidth = SAMPLE_SYS_ALIGN_WIDTH;
    s32ret = HI_MPI_SYS_SetConf(&stSysConf);
    if (HI_SUCCESS != s32ret)
    {
        SAMPLE_PRT("HI_MPI_SYS_SetConf failed\n");
        return HI_FAILURE;
    }

    s32ret = HI_MPI_SYS_Init();
    if (HI_SUCCESS != s32ret)
    {
        SAMPLE_PRT("HI_MPI_SYS_Init failed!\n");
        return HI_FAILURE;
    }

    return HI_SUCCESS;
}

HI_S32 HISA_INNER_CODEC_CfgAudio(AUDIO_SAMPLE_RATE_E enSample)
{
	HI_S32 fdAcodec = -1;
	HI_S32 ret = HI_SUCCESS;
	unsigned int i2s_fs_sel = 0;
	int iAcodecInputVol = 0;

	fdAcodec = open(ACODEC_FILE, O_RDWR);
	if (fdAcodec < 0)
	{
		LMLOG(LERR, "%s: can't open Acodec,%s\n", __FUNCTION__, ACODEC_FILE);
		return HI_FAILURE;
	}
	if (ioctl(fdAcodec, ACODEC_SOFT_RESET_CTRL))
	{
		LMLOG(LERR, "Reset audio codec error\n");
	}

	if ((AUDIO_SAMPLE_RATE_8000 == enSample)
		|| (AUDIO_SAMPLE_RATE_11025 == enSample)
		|| (AUDIO_SAMPLE_RATE_12000 == enSample))
	{
		i2s_fs_sel = 0x18;
	}
	else if ((AUDIO_SAMPLE_RATE_16000 == enSample)
			 || (AUDIO_SAMPLE_RATE_22050 == enSample)
			 || (AUDIO_SAMPLE_RATE_24000 == enSample))
	{
		i2s_fs_sel = 0x19;
	}
	else if ((AUDIO_SAMPLE_RATE_32000 == enSample)
			 || (AUDIO_SAMPLE_RATE_44100 == enSample)
			 || (AUDIO_SAMPLE_RATE_48000 == enSample))
	{
		i2s_fs_sel = 0x1a;
	}
	else
	{
		LMLOG(LERR, "%s: not support enSample:%d\n", __FUNCTION__, enSample);
		ret = HI_FAILURE;
	}

	if (ioctl(fdAcodec, ACODEC_SET_I2S1_FS, &i2s_fs_sel))
	{
		LMLOG(LERR, "%s: set acodec sample rate failed\n", __FUNCTION__);
		ret = HI_FAILURE;
	}

	if (0) /* should be 1 when micin */
	{
		/******************************************************************************************
		The input volume range is [-87, +86]. Both the analog gain and digital gain are adjusted.
		A larger value indicates higher volume.
		For example, the value 86 indicates the maximum volume of 86 dB,
		and the value -87 indicates the minimum volume (muted status).
		The volume adjustment takes effect simultaneously in the audio-left and audio-right channels.
		The recommended volume range is [+10, +56].
		Within this range, the noises are lowest because only the analog gain is adjusted,
		and the voice quality can be guaranteed.
		*******************************************************************************************/
		iAcodecInputVol = 30;
		if (ioctl(fdAcodec, ACODEC_SET_INPUT_VOL, &iAcodecInputVol))
		{
			LMLOG(LERR, "%s: set acodec micin volume failed\n", __FUNCTION__);
			return HI_FAILURE;
		}

	}

	close(fdAcodec);

	return ret;
}

/******************************************************************************
function : Ao bind Adec
 ******************************************************************************/
HI_S32 HISA_COMM_AUDIO_AoBindAdec(AUDIO_DEV AoDev, AO_CHN AoChn, ADEC_CHN AdChn)
{
    MPP_CHN_S stSrcChn, stDestChn;

    stSrcChn.enModId = HI_ID_ADEC;
    stSrcChn.s32DevId = 0;
    stSrcChn.s32ChnId = AdChn;
    stDestChn.enModId = HI_ID_AO;
    stDestChn.s32DevId = AoDev;
    stDestChn.s32ChnId = AoChn;

    return HI_MPI_SYS_Bind(&stSrcChn, &stDestChn);
}

/* config codec */
HI_S32 HISA_COMM_AUDIO_CfgAcodec(AIO_ATTR_S* pstAioAttr)
{
    HI_S32 s32Ret = HI_SUCCESS;
    HI_BOOL bCodecCfg = HI_FALSE;

    /*** INNER AUDIO CODEC ***/
    s32Ret = HISA_INNER_CODEC_CfgAudio(pstAioAttr->enSamplerate);
    if (HI_SUCCESS != s32Ret)
    {
        LMLOG(LERR, "%s:SAMPLE_INNER_CODEC_CfgAudio failed", __FUNCTION__);
        return s32Ret;
    }
    bCodecCfg = HI_TRUE;

    if (!bCodecCfg)
    {
        LMLOG(LERR, "Can not find the right codec.");
        return HI_FALSE;
    }
    return HI_SUCCESS;
}

/******************************************************************************
* funciton : Get enSize by diffrent sensor
******************************************************************************/
HI_S32 HISA_COMM_VI_GetSizeBySensor(PIC_SIZE_E* penSize)
{
    HI_S32 s32Ret = HI_SUCCESS;
    SAMPLE_VI_MODE_E enMode = SENSOR_TYPE;

    if (!penSize)
    {
        return HI_FAILURE;
    }

    switch (enMode)
    {
        case PANASONIC_MN34220_SUBLVDS_720P_120FPS:
        case PANASONIC_MN34220_MIPI_720P_120FPS:
        case SONY_IMX117_LVDS_720P_30FPS:
            *penSize = PIC_HD720;
            break;
        case PANASONIC_MN34220_SUBLVDS_1080P_30FPS:
        case PANASONIC_MN34220_MIPI_1080P_30FPS:
        case OMNIVISION_OV4689_MIPI_1080P_30FPS:
        case APTINA_AR0330_MIPI_1080P_30FPS:
        case SONY_IMX178_LVDS_1080P_30FPS:
        case SONY_IMX185_MIPI_1080P_30FPS:
        case SONY_IMX117_LVDS_1080P_30FPS:
        case APTINA_AR0230_HISPI_1080P_30FPS:
        case APTINA_AR0237_HISPI_1080P_30FPS:     
            *penSize = PIC_HD1080;
            break;
        case APTINA_AR0330_MIPI_1536P_25FPS:
        case SONY_IMX123_LVDS_QXGA_30FPS:
            *penSize = PIC_QXGA;
            break;
        case APTINA_AR0330_MIPI_1296P_25FPS:
            *penSize = PIC_2304x1296;
            break;
        case OMNIVISION_OV4689_MIPI_4M_30FPS:
            *penSize = PIC_2592x1520;
            break;
        case SONY_IMX178_LVDS_5M_30FPS:
        case OMNIVISION_OV5658_MIPI_5M_30FPS:
            *penSize = PIC_5M;
            break;

        default:
            break;
    }

    return s32Ret;
}

/*****************************************************************************
function : get picture size(w*h), according Norm and enPicSize
******************************************************************************/
HI_S32 HISA_COMM_SYS_GetPicSize(VIDEO_NORM_E enNorm, PIC_SIZE_E enPicSize, SIZE_S* pstSize)
{
    switch (enPicSize)
    {
        case PIC_QCIF:
            pstSize->u32Width  = 176;
            pstSize->u32Height = (VIDEO_ENCODING_MODE_PAL == enNorm) ? 144 : 120;
            break;
        case PIC_CIF:
            pstSize->u32Width  = 352;
            pstSize->u32Height = (VIDEO_ENCODING_MODE_PAL == enNorm) ? 288 : 240;
            break;
        case PIC_D1:
            pstSize->u32Width  = 720;
            pstSize->u32Height = (VIDEO_ENCODING_MODE_PAL == enNorm) ? 576 : 480;
            break;
        case PIC_960H:
            pstSize->u32Width  = 960;
            pstSize->u32Height = (VIDEO_ENCODING_MODE_PAL == enNorm) ? 576 : 480;
            break;
        case PIC_2CIF:
            pstSize->u32Width  = 360;
            pstSize->u32Height = (VIDEO_ENCODING_MODE_PAL == enNorm) ? 576 : 480;
            break;
        case PIC_QVGA:    /* 320 * 240 */
            pstSize->u32Width  = 320;
            pstSize->u32Height = 240;
            break;
        case PIC_VGA:     /* 640 * 480 */
            pstSize->u32Width  = 640;
            pstSize->u32Height = 480;
            break;
        case PIC_XGA:     /* 1024 * 768 */
            pstSize->u32Width  = 1024;
            pstSize->u32Height = 768;
            break;
        case PIC_SXGA:    /* 1400 * 1050 */
            pstSize->u32Width  = 1400;
            pstSize->u32Height = 1050;
            break;
        case PIC_UXGA:    /* 1600 * 1200 */
            pstSize->u32Width  = 1600;
            pstSize->u32Height = 1200;
            break;
        case PIC_QXGA:    /* 2048 * 1536 */
            pstSize->u32Width  = 2048;
            pstSize->u32Height = 1536;
            break;
        case PIC_WVGA:    /* 854 * 480 */
            pstSize->u32Width  = 854;
            pstSize->u32Height = 480;
            break;
        case PIC_WSXGA:   /* 1680 * 1050 */
            pstSize->u32Width = 1680;
            pstSize->u32Height = 1050;
            break;
        case PIC_WUXGA:   /* 1920 * 1200 */
            pstSize->u32Width  = 1920;
            pstSize->u32Height = 1200;
            break;
        case PIC_WQXGA:   /* 2560 * 1600 */
            pstSize->u32Width  = 2560;
            pstSize->u32Height = 1600;
            break;
        case PIC_HD720:   /* 1280 * 720 */
            pstSize->u32Width  = 1280;
            pstSize->u32Height = 720;
            break;
        case PIC_HD1080:  /* 1920 * 1080 */
            pstSize->u32Width  = 1920;
            pstSize->u32Height = 1080;
            break;
        case PIC_2304x1296:  /* 2304 * 1296 */
            pstSize->u32Width  = 2304;
            pstSize->u32Height = 1296;
            break;
        case PIC_2592x1520:  /* 2592 * 1520 */
            pstSize->u32Width  = 2592;
            pstSize->u32Height = 1520;
            break;
        case PIC_5M:      /* 2592 * 1944 */
            pstSize->u32Width  = 2592;
            pstSize->u32Height = 1944;
            break;

        default:
            return HI_FAILURE;
    }
    return HI_SUCCESS;
}

/******************************************************************************
* function : calculate VB Block size of picture.
******************************************************************************/
HI_U32 HISA_COMM_SYS_CalcPicVbBlkSize(VIDEO_NORM_E enNorm, PIC_SIZE_E enPicSize, PIXEL_FORMAT_E enPixFmt, HI_U32 u32AlignWidth)
{
    HI_S32 s32Ret = HI_FAILURE;
    SIZE_S stSize;
    HI_U32 u32VbSize;
    HI_U32 u32HeaderSize;

    s32Ret = HISA_COMM_SYS_GetPicSize(enNorm, enPicSize, &stSize);
    if (HI_SUCCESS != s32Ret)
    {
        SAMPLE_PRT("get picture size[%d] failed!\n", enPicSize);
        return HI_FAILURE;
    }

    if (PIXEL_FORMAT_YUV_SEMIPLANAR_422 != enPixFmt && PIXEL_FORMAT_YUV_SEMIPLANAR_420 != enPixFmt)
    {
        SAMPLE_PRT("pixel format[%d] input failed!\n", enPixFmt);
        return HI_FAILURE;
    }

    if (16 != u32AlignWidth && 32 != u32AlignWidth && 64 != u32AlignWidth)
    {
        SAMPLE_PRT("system align width[%d] input failed!\n", \
                   u32AlignWidth);
        return HI_FAILURE;
    }
    //SAMPLE_PRT("w:%d, u32AlignWidth:%d\n", CEILING_2_POWER(stSize.u32Width,u32AlignWidth), u32AlignWidth);
    u32VbSize = (CEILING_2_POWER(stSize.u32Width, u32AlignWidth) * \
                 CEILING_2_POWER(stSize.u32Height, u32AlignWidth) * \
                 ((PIXEL_FORMAT_YUV_SEMIPLANAR_422 == enPixFmt) ? 2 : 1.5));

    VB_PIC_HEADER_SIZE(stSize.u32Width, stSize.u32Height, enPixFmt, u32HeaderSize);
    u32VbSize += u32HeaderSize;

    return u32VbSize;
}

/******************************************************************************
* function : Start Ai
******************************************************************************/
HI_S32 HISA_COMM_AUDIO_StartAi(AUDIO_DEV AiDevId, HI_S32 s32AiChnCnt,
                                 AIO_ATTR_S* pstAioAttr, AUDIO_SAMPLE_RATE_E enOutSampleRate, HI_BOOL bResampleEn, HI_VOID* pstAiVqeAttr, HI_U32 u32AiVqeType)
{
    HI_S32 i;
    HI_S32 s32ret;

    s32ret = HI_MPI_AI_SetPubAttr(AiDevId, pstAioAttr);
    if (s32ret)
    {
        LMLOG(LERR, "%s: HI_MPI_AI_SetPubAttr(%d) failed with %#x\n", __FUNCTION__, AiDevId, s32ret);
        return s32ret;
    }

    s32ret = HI_MPI_AI_Enable(AiDevId);
    if (s32ret)
    {
        LMLOG(LERR, "%s: HI_MPI_AI_Enable(%d) failed with %#x\n", __FUNCTION__, AiDevId, s32ret);
        return s32ret;
    }

    for (i = 0; i < s32AiChnCnt; i++)
    {
        s32ret = HI_MPI_AI_EnableChn(AiDevId, i/(pstAioAttr->enSoundmode + 1));
        if (s32ret)
        {
            LMLOG(LERR, "%s: HI_MPI_AI_EnableChn(%d,%d) failed with %#x\n", __FUNCTION__, AiDevId, i, s32ret);
            return s32ret;
        }

        if (HI_TRUE == bResampleEn)
        {
            s32ret = HI_MPI_AI_EnableReSmp(AiDevId, i, enOutSampleRate);
            if (s32ret)
            {
                LMLOG(LERR, "%s: HI_MPI_AI_EnableReSmp(%d,%d) failed with %#x\n", __FUNCTION__, AiDevId, i, s32ret);
                return s32ret;
            }
        }

        if (NULL != pstAiVqeAttr)
        {
			HI_BOOL bAiVqe = HI_TRUE;
			switch (u32AiVqeType)
            {
				case 0:
                    s32ret = HI_SUCCESS;
					bAiVqe = HI_FALSE;
                    break;
                case 1:
                    s32ret = HI_MPI_AI_SetVqeAttr(AiDevId, i, SAMPLE_AUDIO_AO_DEV, i, (AI_VQE_CONFIG_S *)pstAiVqeAttr);
                    break;
                case 2:
                    s32ret = HI_MPI_AI_SetHiFiVqeAttr(AiDevId, i, (AI_HIFIVQE_CONFIG_S *)pstAiVqeAttr);
                    break;
                default:
                    s32ret = HI_FAILURE;
                    break;
            }

            if (s32ret)
            {
                LMLOG(LERR, "%s: SetAiVqe%d(%d,%d) failed with %#x\n", __FUNCTION__, u32AiVqeType, AiDevId, i, s32ret);
                return s32ret;
            }

		    if (bAiVqe)
            {
                s32ret = HI_MPI_AI_EnableVqe(AiDevId, i);
	            if (s32ret)
	            {
	                LMLOG(LERR, "%s: HI_MPI_AI_EnableVqe(%d,%d) failed with %#x\n", __FUNCTION__, AiDevId, i, s32ret);
	                return s32ret;
	            }
            }
        }
    }

    return HI_SUCCESS;
}

/******************************************************************************
* function : Start Adec
******************************************************************************/
HI_S32 HISA_COMM_AUDIO_StartAdec(ADEC_CHN AdChn, PAYLOAD_TYPE_E enType)
{
    HI_S32 s32ret;
    ADEC_CHN_ATTR_S stAdecAttr;
    ADEC_ATTR_ADPCM_S stAdpcm;
    ADEC_ATTR_G711_S stAdecG711;
    ADEC_ATTR_G726_S stAdecG726;
    ADEC_ATTR_LPCM_S stAdecLpcm;

    stAdecAttr.enType = enType;
    stAdecAttr.u32BufSize = 20;
    stAdecAttr.enMode = ADEC_MODE_STREAM;/* propose use pack mode in your app */

    if (PT_ADPCMA == stAdecAttr.enType)
    {
        stAdecAttr.pValue = &stAdpcm;
        stAdpcm.enADPCMType = AUDIO_ADPCM_TYPE ;
    }
    else if (PT_G711A == stAdecAttr.enType || PT_G711U == stAdecAttr.enType)
    {
        stAdecAttr.pValue = &stAdecG711;
    }
    else if (PT_G726 == stAdecAttr.enType)
    {
        stAdecAttr.pValue = &stAdecG726;
        stAdecG726.enG726bps = G726_BPS ;
    }
    else if (PT_LPCM == stAdecAttr.enType)
    {
        stAdecAttr.pValue = &stAdecLpcm;
        stAdecAttr.enMode = ADEC_MODE_PACK;/* lpcm must use pack mode */
    }
    else
    {
        LMLOG(LERR, "%s: invalid aenc payload type:%d\n", __FUNCTION__, stAdecAttr.enType);
        return HI_FAILURE;
    }

    /* create adec chn*/
    s32ret = HI_MPI_ADEC_CreateChn(AdChn, &stAdecAttr);
    if (HI_SUCCESS != s32ret)
    {
        LMLOG(LERR, "%s: HI_MPI_ADEC_CreateChn(%d) failed with %#x!\n", __FUNCTION__, \
               AdChn, s32ret);
        return s32ret;
    }
    return 0;
}

/******************************************************************************
 * function : Start Ao
 ******************************************************************************/
HI_S32 HISA_COMM_AUDIO_StartAo(AUDIO_DEV AoDevId, HI_S32 s32AoChnCnt,
                                 AIO_ATTR_S* pstAioAttr, AUDIO_SAMPLE_RATE_E enInSampleRate, HI_BOOL bResampleEn, HI_VOID* pstAoVqeAttr, HI_U32 u32AoVqeType)
{
    HI_S32 i;
    HI_S32 s32ret;

    s32ret = HI_MPI_AO_SetPubAttr(AoDevId, pstAioAttr);
    if (HI_SUCCESS != s32ret)
    {
        LMLOG(LERR, "%s: HI_MPI_AO_SetPubAttr(%d) failed with %#x!\n", __FUNCTION__, \
               AoDevId, s32ret);
        return HI_FAILURE;
    }

    s32ret = HI_MPI_AO_Enable(AoDevId);
    if (HI_SUCCESS != s32ret)
    {
        LMLOG(LERR, "%s: HI_MPI_AO_Enable(%d) failed with %#x!\n", __FUNCTION__, AoDevId, s32ret);
        return HI_FAILURE;
    }

    for (i = 0; i < s32AoChnCnt; i++)
    {
        s32ret = HI_MPI_AO_EnableChn(AoDevId, i/(pstAioAttr->enSoundmode + 1));
        if (HI_SUCCESS != s32ret)
        {
            LMLOG(LERR, "%s: HI_MPI_AO_EnableChn(%d) failed with %#x!\n", __FUNCTION__, i, s32ret);
            return HI_FAILURE;
        }

        if (HI_TRUE == bResampleEn)
        {
            s32ret = HI_MPI_AO_DisableReSmp(AoDevId, i);
            s32ret |= HI_MPI_AO_EnableReSmp(AoDevId, i, enInSampleRate);
            if (HI_SUCCESS != s32ret)
            {
                LMLOG(LERR, "%s: HI_MPI_AO_EnableReSmp(%d,%d) failed with %#x!\n", __FUNCTION__, AoDevId, i, s32ret);
                return HI_FAILURE;
            }
        }

		if (NULL != pstAoVqeAttr)
        {
			HI_BOOL bAoVqe = HI_TRUE;
			switch (u32AoVqeType)
            {
				case 0:
                    s32ret = HI_SUCCESS;
					bAoVqe = HI_FALSE;
                    break;
                case 1:
                    s32ret = HI_MPI_AO_SetVqeAttr(AoDevId, i, (AO_VQE_CONFIG_S *)pstAoVqeAttr);
                    break;
                default:
                    s32ret = HI_FAILURE;
                    break;
            }

            if (s32ret)
            {
                LMLOG(LERR, "%s: SetAoVqe%d(%d,%d) failed with %#x\n", __FUNCTION__, u32AoVqeType, AoDevId, i, s32ret);
                return s32ret;
            }

		    if (bAoVqe)
            {
                s32ret = HI_MPI_AO_EnableVqe(AoDevId, i);
	            if (s32ret)
	            {
	                LMLOG(LERR, "%s: HI_MPI_AI_EnableVqe(%d,%d) failed with %#x\n", __FUNCTION__, AoDevId, i, s32ret);
	                return s32ret;
	            }
            }
        }
    }

    return HI_SUCCESS;
}

static void hisa_card_detect(AudSndCard **card)
{
#if 0
    HI_S32 s32ret = HI_SUCCESS;
	AUDIO_DEV AiDevId = SAMPLE_AUDIO_AI_DEV, AoDevId = SAMPLE_AUDIO_AO_DEV;

    HI_CHAR ch;
    HI_BOOL bExit = HI_FALSE;
    VB_CONF_S stVbConf;

    AO_CHN      AoChn = 0;
	ADEC_CHN    AdChn = 0;
    HI_S32      s32AoChnCnt;
    AIO_ATTR_S stAioAttr;
    HI_S32 s32MilliSec = 10;
    AUDIO_FRAME_S pstDataFrame;
    HI_S32      s32AiChnCnt;
    HI_S32      s32Volume = 6;
    HI_S32      s32VolumeDb = 10;

    stAioAttr.enSamplerate   = AUDIO_SAMPLE_RATE_8000;
    stAioAttr.enBitwidth     = AUDIO_BIT_WIDTH_16;
    stAioAttr.enWorkmode     = AIO_MODE_I2S_MASTER;
    stAioAttr.enSoundmode    = AUDIO_SOUND_MODE_MONO;
    stAioAttr.u32EXFlag      = 0;
    stAioAttr.u32FrmNum      = 30;
    stAioAttr.u32PtNumPerFrm = SAMPLE_AUDIO_PTNUMPERFRM;
    stAioAttr.u32ChnCnt      = 1;
    stAioAttr.u32ClkSel      = 0;

	PIC_SIZE_E enSize[3]= {PIC_HD1080, PIC_HD1080, PIC_D1};	
	HI_U32 u32BlkSize;

    memset(&stVbConf, 0, sizeof(VB_CONF_S));

	/***************************************************************************************************/

	/******************************************
     step  1: init sys variable
    ******************************************/
    memset(&stVbConf, 0, sizeof(VB_CONF_S));

    HISA_COMM_VI_GetSizeBySensor(&enSize[0]);

    stVbConf.u32MaxPoolCnt = 128;

    /*video buffer*/
    u32BlkSize = HISA_COMM_SYS_CalcPicVbBlkSize(gs_enNorm, \
                 enSize[0], SAMPLE_PIXEL_FORMAT, SAMPLE_SYS_ALIGN_WIDTH);
    stVbConf.astCommPool[0].u32BlkSize = u32BlkSize;
    stVbConf.astCommPool[0].u32BlkCnt = 12;

	/***************************************************************************************************/

    s32ret = HISA_COMM_SYS_Init(&stVbConf);
    if (HI_SUCCESS != s32ret)
    {
        LMLOG(LERR, "%s: system init failed with %d!", __FUNCTION__, s32ret);
        return;
    }

    /* config internal audio codec */
    s32ret = HISA_COMM_AUDIO_CfgAcodec(&stAioAttr);
    if (HI_SUCCESS != s32ret)
    {
		LMLOG(LERR, "%s:Hisa comm audio cfgAcodec  %d!", __FUNCTION__, s32ret);
        return HI_FAILURE;
    }
#endif

    *card = (AudSndCard *)malloc(sizeof(AudSndCard));

    LMLOG(LINF, "%s: Register snd card success!", __FUNCTION__);

    return;
}

static void hisa_card_read_preprocess(void)
{
    HI_S32 s32ret = HI_SUCCESS;
	AUDIO_DEV AiDevId = SAMPLE_AUDIO_AI_DEV, AoDevId = SAMPLE_AUDIO_AO_DEV;

    HI_CHAR ch;
    HI_BOOL bExit = HI_FALSE;
    VB_CONF_S stVbConf;

    AO_CHN      AoChn = 0;
	ADEC_CHN    AdChn = 0;
    HI_S32      s32AoChnCnt;
    AIO_ATTR_S stAioAttr;
    HI_S32 s32MilliSec = 10;
    AUDIO_FRAME_S pstDataFrame;
    HI_S32      s32AiChnCnt;
    HI_S32      s32Volume = 6;
    HI_S32      s32VolumeDb = 10;

    stAioAttr.enSamplerate   = AUDIO_SAMPLE_RATE_8000;
    stAioAttr.enBitwidth     = AUDIO_BIT_WIDTH_16;
    stAioAttr.enWorkmode     = AIO_MODE_I2S_MASTER;
    stAioAttr.enSoundmode    = AUDIO_SOUND_MODE_MONO;
    stAioAttr.u32EXFlag      = 0;
    stAioAttr.u32FrmNum      = 30;
    stAioAttr.u32PtNumPerFrm = SAMPLE_AUDIO_PTNUMPERFRM;
    stAioAttr.u32ChnCnt      = 1;
    stAioAttr.u32ClkSel      = 0;

	PIC_SIZE_E enSize[3]= {PIC_HD1080, PIC_HD1080, PIC_D1};	
	HI_U32 u32BlkSize;

    memset(&stVbConf, 0, sizeof(VB_CONF_S));

    /********************************************
	 *       start Ai
	 *********************************************/
	s32AiChnCnt = stAioAttr.u32ChnCnt;
	s32ret = HISA_COMM_AUDIO_StartAi(AiDevId, s32AiChnCnt, &stAioAttr, enOutSampleRate, gs_bAioReSample, NULL, 0);
	if (s32ret != HI_SUCCESS){
		LMLOG(LERR, "%s: start Ai failed with %d!\n", __FUNCTION__, s32ret);
		return;
	}


	s32ret = HI_MPI_AI_GetPubAttr(AiDevId, &AiDevAttr);
	if(HI_SUCCESS != s32ret){
		LMLOG(LERR, "get ai %d attr err:0x%x\n", AiDevId, s32ret);
		return;
	}

}

static void hisa_card_init(SndCardDesc *desc, AudSndCard *card)
{
    //create reader
    card->reader = NULL;
    (desc->create_reader)(card);

    //create writer
    card->writer = NULL;
    (desc->create_writer)(card);

    card->desc = desc;

    LMLOG(LINF, "%s: Init snd card success!", __FUNCTION__);

    return;
}

static void hisa_card_create_reader(AudSndCard *card)
{
    if(NULL == card->reader){
        card->reader = (Reader *)malloc(sizeof(Reader));
        card->reader->snd_read = snd_read;
        LMLOG(LINF, "%s: The Reader Register Success!", __FUNCTION__);
    }else{
        LMLOG(LINF, "%s: The Reader haved Register!", __FUNCTION__);
        return;
    }

    LMLOG(LINF, "%s: Create snd card Reader success!", __FUNCTION__);

    return;
}

static void hisa_card_create_writer(AudSndCard *card)
{
    if(NULL == card->writer){
        card->writer = (Writer *)malloc(sizeof(Writer));
        card->writer->snd_write = snd_write;
        LMLOG(LINF, "%s: The Writer Register Success!", __FUNCTION__);
    }else{
        LMLOG(LINF, "%s: The Write haved Register!", __FUNCTION__);
        return;
    }

    LMLOG(LINF, "%s: Create snd card Writer success!", __FUNCTION__);

    return;
}

static void hisa_card_unload(AudSndCard *card)
{
    LMLOG(LINF, "%s: Unload snd card success!", __FUNCTION__);

    return;
}

/******************************************************************************
* function : Stop Ai
******************************************************************************/
HI_S32 HISA_COMM_AUDIO_StopAi(AUDIO_DEV AiDevId, HI_S32 s32AiChnCnt,
                                HI_BOOL bResampleEn, HI_BOOL bVqeEn)
{
    HI_S32 i;
    HI_S32 s32Ret;

    for (i = 0; i < s32AiChnCnt; i++)
    {
        if (HI_TRUE == bResampleEn)
        {
            s32Ret = HI_MPI_AI_DisableReSmp(AiDevId, i);
            if (HI_SUCCESS != s32Ret)
            {
                printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "failed");
                return s32Ret;
            }
        }

        if (HI_TRUE == bVqeEn)
        {
            s32Ret = HI_MPI_AI_DisableVqe(AiDevId, i);
            if (HI_SUCCESS != s32Ret)
            {
                printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "failed");
                return s32Ret;
            }
        }

        s32Ret = HI_MPI_AI_DisableChn(AiDevId, i);
        if (HI_SUCCESS != s32Ret)
        {
            printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "failed");
            return s32Ret;
        }
    }

    s32Ret = HI_MPI_AI_Disable(AiDevId);
    if (HI_SUCCESS != s32Ret)
    {
        printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "failed");
        return s32Ret;
    }

    return HI_SUCCESS;
}

/******************************************************************************
* function : Stop Ao
******************************************************************************/
HI_S32 HISA_COMM_AUDIO_StopAo(AUDIO_DEV AoDevId, HI_S32 s32AoChnCnt, HI_BOOL bResampleEn, HI_BOOL bVqeEn)
{
    HI_S32 i;
    HI_S32 s32Ret;

    for (i = 0; i < s32AoChnCnt; i++)
    {
        if (HI_TRUE == bResampleEn)
        {
            s32Ret = HI_MPI_AO_DisableReSmp(AoDevId, i);
            if (HI_SUCCESS != s32Ret)
            {
                printf("%s: HI_MPI_AO_DisableReSmp failed with %#x!\n", __FUNCTION__, s32Ret);
                return s32Ret;
            }
        }

		if (HI_TRUE == bVqeEn)
        {
            s32Ret = HI_MPI_AO_DisableVqe(AoDevId, i);
            if (HI_SUCCESS != s32Ret)
            {
                printf("[Func]:%s [Line]:%d [Info]:%s\n", __FUNCTION__, __LINE__, "failed");
                return s32Ret;
            }
        }

        s32Ret = HI_MPI_AO_DisableChn(AoDevId, i);
        if (HI_SUCCESS != s32Ret)
        {
            printf("%s: HI_MPI_AO_DisableChn failed with %#x!\n", __FUNCTION__, s32Ret);
            return s32Ret;
        }
    }

    s32Ret = HI_MPI_AO_Disable(AoDevId);
    if (HI_SUCCESS != s32Ret)
    {
        printf("%s: HI_MPI_AO_Disable failed with %#x!\n", __FUNCTION__, s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

/******************************************************************************
* function : Ao unbind Adec
******************************************************************************/
HI_S32 HISA_COMM_AUDIO_AoUnbindAdec(AUDIO_DEV AoDev, AO_CHN AoChn, ADEC_CHN AdChn)
{
    MPP_CHN_S stSrcChn, stDestChn;

    stSrcChn.enModId = HI_ID_ADEC;
    stSrcChn.s32ChnId = AdChn;
    stSrcChn.s32DevId = 0;
    stDestChn.enModId = HI_ID_AO;
    stDestChn.s32DevId = AoDev;
    stDestChn.s32ChnId = AoChn;

    return HI_MPI_SYS_UnBind(&stSrcChn, &stDestChn);
}

/******************************************************************************
* function : Stop Adec
******************************************************************************/
HI_S32 HISA_COMM_AUDIO_StopAdec(ADEC_CHN AdChn)
{
    HI_S32 s32Ret;

    s32Ret = HI_MPI_ADEC_DestroyChn(AdChn);
    if (HI_SUCCESS != s32Ret)
    {
        printf("%s: HI_MPI_ADEC_DestroyChn(%d) failed with %#x!\n", __FUNCTION__,
               AdChn, s32Ret);
        return s32Ret;
    }

    return HI_SUCCESS;
}

static void hisa_card_read_postprocess(void)
{
	AI_CHN      AiChn = 0;
	AO_CHN      AoChn = 0;
	ADEC_CHN    AdChn = 0;
    int         AoDevId = 0;
    int         AiDevId = 0;
	int s32Ret = 0;
    int s32AiChnCnt = 1;
    int s32AoChnCnt = 1;

	s32Ret = HISA_COMM_AUDIO_StopAi(AiDevId, s32AiChnCnt, gs_bAioReSample, HI_FALSE);
    if (s32Ret != HI_SUCCESS)
    {
        LMLOG(LERR, "%s: Hisa stop Ai failed is %d.", __FUNCTION__, s32Ret);
        return HI_FAILURE;
    }
}

static void hisa_card_write_preprocess(void)
{
    HI_S32 s32ret = HI_SUCCESS;
	AUDIO_DEV AiDevId = SAMPLE_AUDIO_AI_DEV, AoDevId = SAMPLE_AUDIO_AO_DEV;

    HI_CHAR ch;
    HI_BOOL bExit = HI_FALSE;
    VB_CONF_S stVbConf;

    AO_CHN      AoChn = 0;
	ADEC_CHN    AdChn = 0;
    HI_S32      s32AoChnCnt;
    AIO_ATTR_S stAioAttr;
    HI_S32 s32MilliSec = 10;
    AUDIO_FRAME_S pstDataFrame;
    HI_S32      s32AiChnCnt;
    HI_S32      s32VolumeDb = 6;

    stAioAttr.enSamplerate   = AUDIO_SAMPLE_RATE_8000;
    stAioAttr.enBitwidth     = AUDIO_BIT_WIDTH_16;
    stAioAttr.enWorkmode     = AIO_MODE_I2S_MASTER;
    stAioAttr.enSoundmode    = AUDIO_SOUND_MODE_MONO;
    stAioAttr.u32EXFlag      = 0;
    stAioAttr.u32FrmNum      = 30;
    stAioAttr.u32PtNumPerFrm = SAMPLE_AUDIO_PTNUMPERFRM;
    stAioAttr.u32ChnCnt      = 1;
    stAioAttr.u32ClkSel      = 0;

	PIC_SIZE_E enSize[3]= {PIC_HD1080, PIC_HD1080, PIC_D1};	
	HI_U32 u32BlkSize;

    memset(&stVbConf, 0, sizeof(VB_CONF_S));

    s32ret = HISA_COMM_AUDIO_StartAdec(AdChn, gs_enPayloadType);
    if (s32ret != HI_SUCCESS)
    {
		LMLOG(LERR, "%s: start Adec failed with %d!", __FUNCTION__, s32ret);
        return;
    }

    /********************************************
	 *       start Ao
	 *********************************************/
    s32AoChnCnt = stAioAttr.u32ChnCnt;
    s32ret = HISA_COMM_AUDIO_StartAo(AoDevId, s32AoChnCnt, &stAioAttr, enInSampleRate, gs_bAioReSample, NULL, 0);
    if (s32ret != HI_SUCCESS)
    {
		LMLOG(LERR, "%s: start Ao failed with %d!\n", __FUNCTION__, s32ret);
        return;
    }

	s32ret = HISA_COMM_AUDIO_AoBindAdec(AoDevId, AoChn, AdChn);
    if (s32ret != HI_SUCCESS)
    {
		LMLOG(LERR, "%s: Ao bind Adec failed with %d!\n", __FUNCTION__, s32ret);
        return HI_FAILURE;
    }

    s32ret = HI_MPI_AO_GetPubAttr(AoDevId, &AoDevAttr);
	if(HI_SUCCESS != s32ret){
		LMLOG(LERR, "%s: Get ao %d attr err:0x%x\n", __FUNCTION__,  AoDevId, s32ret);
		return;
	}

    s32ret = HI_MPI_AO_SetVolume(AoDevId, s32VolumeDb);
    if(HI_SUCCESS != s32ret){
		LMLOG(LERR, "%s: Set ao %d Volume err:0x%x\n", __FUNCTION__,  AoDevId, s32ret);
		return;
	}
}

static void hisa_card_write_postprocess(void)
{
    AI_CHN      AiChn = 0;
	AO_CHN      AoChn = 0;
	ADEC_CHN    AdChn = 0;
    int         AoDevId = 0;
    int         AiDevId = 0;
	int s32Ret = 0;
    int s32AiChnCnt = 1;
    int s32AoChnCnt = 1;

    s32Ret = HISA_COMM_AUDIO_StopAdec(AdChn);
    if (s32Ret != HI_SUCCESS)
    {
        LMLOG(LERR, "%s: Hisa stop Adec failed is %d.", __FUNCTION__, s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HISA_COMM_AUDIO_StopAo(AoDevId, s32AoChnCnt, gs_bAioReSample, HI_FALSE);
    if (s32Ret != HI_SUCCESS)
    {
        LMLOG(LERR, "%s: Hisa stop Ao failed is %d.", __FUNCTION__, s32Ret);
        return HI_FAILURE;
    }

    s32Ret = HISA_COMM_AUDIO_AoUnbindAdec(AoDevId, AoChn, AdChn);
    if (s32Ret != HI_SUCCESS)
    {
        LMLOG(LERR, "%s: Hisa stop Ao unbind Adec failed is %d.", __FUNCTION__, s32Ret);
        return HI_FAILURE;
    }
}

SndCardDesc InterCom_hisa_card_desc = {
    .detect = hisa_card_detect,
    .init   = hisa_card_init,
    .read_preprocess  = hisa_card_read_preprocess,
    .read_postprocess = hisa_card_read_postprocess,
    .create_reader = hisa_card_create_reader,
    .create_writer = hisa_card_create_writer,
    .write_preprocess  = hisa_card_write_preprocess,
    .write_postprocess = hisa_card_write_postprocess,
    .unload = hisa_card_unload
};
