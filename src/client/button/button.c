#include <pthread.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

#include "log/lmlog.h"
#include "audio/aud.h"
#include "audio/encode.h"
#include "defs.h"


extern Recorder *recorder;

#define KEY_NUM 10
#define KEY_GPIO9_4_DOWN 0x09
#define KEY_GPIO9_4_UP   0x59

int fd;
unsigned char key_array[KEY_NUM] = {0};

int calls_status(void)
{
    return GOOD;
}

int terminal_calls(void)
{
    return GOOD;
}

int process_button_event(int event)
{
    int status,ret;

    //judgement is knob down or release button
    if(event == DOWN){
        //judgement the linphone calls is running or not
        status = calls_status();
        if(status == RUNNING){
            terminal_calls();
        }

        recorder_setup();
        pthread_create(recorder->thread_id, NULL, (void *)snd_record_start, NULL);
    }

    if(event == UP){
        snd_record_stop();
        usleep(1000);
        snd_encode_start(recorder->aud_raw_fp, recorder->aud_encode_fp);
        ret = snd_ftp_put(recorder->aud_encode_fp);
        if(ret != GOOD){
            LMLOG(LERR, "%s:send aud file failed ret = %d.", ret);
            return ret;
        }
    }

    return ret;
}

void signal_f(int signum)
{
    static unsigned int cnt = 1;
    unsigned int i;
    int event;

    read(fd, &key_array, sizeof(key_array));

    LMLOG(LINF, "%s: The key count:%4d\t",__FUNCTION__, cnt++);
    for(i=0; i<KEY_NUM; i++){
        printf(" %02x ", key_array[i]);
    }
    printf("\n");

    if(key_array[9] == KEY_GPIO9_4_DOWN){
        event = DOWN;
        LMLOG(LDBG_1, "%s: The button event is %d", __FUNCTION__, event);
        process_button_event(event);
    }

    if(key_array[9] == KEY_GPIO9_4_UP){
        event = UP;
        LMLOG(LDBG_1, "%s: The button event is %d", __FUNCTION__, event);
        process_button_event(event);
    }

}

int button_setup(void)
{
	int flag;

    fd = open("/dev/buttons", O_RDWR);
    if (fd < 0){
        LMLOG(LERR, "%s: Open the buttons device failed!", __FUNCTION__);
        return BAD;
    }
    signal(SIGIO, signal_f);
    fcntl(fd, F_SETOWN, getpid());
    flag = fcntl(fd, F_GETFL);
    fcntl(fd, F_SETFL, flag|FASYNC);

    return GOOD;
}
