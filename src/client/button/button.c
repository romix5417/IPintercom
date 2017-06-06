#include <pthread.h>
#include <stdio.h>

#include "log/lmlog.h"
#include "audio/aud.h"
#include "audio/encode.h"
#include "defs.h"


extern Recorder *recorder;

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
    }else{
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

int button_setup(void)
{
    return GOOD;
}
