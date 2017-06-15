#include <pthread.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

#include "log/lmlog.h"
#include "audio/aud.h"
#include "audio/encode.h"
#include "socket/sockets-util.h"
#include "client/client_function.h"
#include "defs.h"


extern Recorder *recorder;
extern int DEV_NUM;
extern int HOST_NUM;

#define FILENAME_LEN 16

#define KEY_NUM 1
#define KEY_GPIO9_4_DOWN 0x00
#define KEY_GPIO9_4_UP   0x80

#define HOST_IP "192.168.10.101"
#define PORT 9999

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
    int status = 0;
    int ret;
    int cnt = 3;

    uint8_t packet[24] = {0};
    int packet_len = 24;
    IPinterCom_control_hdr *ip_hdr = (IPinterCom_control_hdr *)packet;
    ip_addr_t dest_ip;
    int remote_port = PORT;


    //judgement is knob down or release button
    if(event == DOWN){
        //judgement the linphone calls is running or not
        status = calls_status();
        if(status == RUNNING){
            //terminal_calls();
            return status;
        }

        recorder_setup();
        pthread_create(&recorder->thread_id, NULL, (void *)snd_record_start, NULL);
    }

    if(event == UP){
        if(recorder != NULL){
            snd_record_stop();

            while(recorder->state != STOPPING){
                LMLOG(LINF, "%s: Waiting the recorder stop...", __FUNCTION__);
                usleep(100000);
            }

            if(recorder->aud_raw_fp != NULL){
                LMLOG(LINF, "%s: Close the stream file fp.", __FUNCTION__);
                fclose(recorder->aud_raw_fp);
                recorder->aud_raw_fp = NULL;
                ret = snd_ftp_put(recorder->aud_encode_file);
                if(ret != GOOD){
                    LMLOG(LERR, "%s:Send aud file failed ret = %d.", ret);
                    return ret;
                }

                memset(recorder->aud_encode_file, 0, FILENAME_LEN);
            }

            recorder_destroy();

            ip_hdr->src  = DEV_NUM;
            ip_hdr->dest = HOST_NUM;
            ip_hdr->cmd = PUT_CMD;

            dest_ip.afi  = AF_INET;
            dest_ip.addr.v4.s_addr =  inet_addr(HOST_IP);
            do{
                ret = send_cmd_packet(packet,packet_len, dest_ip,remote_port);
                if(ret == 0){
                    break;
                }else{
                    cnt--;
                    LMLOG(LWRN, "%s: Send cmd failed. The error is %d.", __FUNCTION__, ret);
                }
            }while(cnt);
        }
    }

    return ret;
}

int event = UP;

void signal_f(int signum)
{
    static unsigned int cnt = 1;
    unsigned int i;

    read(fd, &key_array, sizeof(key_array));

    LMLOG(LINF, "%s: The key count:%4d\t",__FUNCTION__, cnt++);
    for(i=0; i<KEY_NUM; i++){
        LMLOG(LINF, "The Ket value is %02x ", key_array[i]);
    }

    if(key_array[0] == KEY_GPIO9_4_DOWN && event == UP){
        event = DOWN;
        LMLOG(LDBG_1, "%s: The button event is %d", __FUNCTION__, event);
        process_button_event(event);
    }

    if(key_array[0] == KEY_GPIO9_4_UP && event == DOWN){
        event = UP;
        LMLOG(LDBG_1, "%s: The button event is %d", __FUNCTION__, event);
        process_button_event(event);
    }

    return;
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
