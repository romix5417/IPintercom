#include <lmlog.h>
#include <audio/aud.h>

#define RUNNING 1
#define DOWN      1
#define RELEASE   0

extern recorder;

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
    int status;

    //judgement is knob down or release button
    if(event == DOWN){
        //judgement the linphone calls is running or not
        status = calls_status();
        if(status == RUNNING){
            terminal_calls();
        }

        recorder_setup();

        pthread_create(recorder->thread_id, snd_record_start, NULL);
    }else{
        snd_record_stop();
        usleep(1000);
        snd_encode_start(recorder->aud_raw_fp, recorder->aud_encode_fp);
        ret = snd_tftp_send(recorder->aud_encode_fp);
    }
}

int button_setup(void)
{
    return GOOD;
}
