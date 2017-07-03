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
#include "transport/transport_function.h"
#include "client/button.h"
#include "cfg/cfg.h"
#include "defs.h"


extern Recorder *recorder;

#define FILENAME_LEN 16

#define KEY_NUM 1
#define KEY_GPIO9_4_DOWN 0x00
#define KEY_GPIO9_4_UP   0x80

extern CFG_OPT global_cfg;

int LINPHONE_STATUS = ON_HOOK;

//#define HOST_IP "192.168.10.244"
//#define PORT 5588

int fd;
unsigned char key_array[KEY_NUM] = {0};
int event = UP;


int get_status(void)
{
    int fp = 0;
    char buffer[] = {"linphonecsh status hook > status.log &"};
    char status[16] = {0};
    int j = 50000000;

    LMLOG(LINF, "%s: Get the status of linphone.", __FUNCTION__);
    fp = popen(buffer,"r");
    if(NULL != fgets(status,13,fp)){
        LMLOG(LINF, "%s: Get the status of linphone : %s, the size:%d.", __FUNCTION__, status, sizeof(status));

        pclose(fp);
        if(0 == memcmp(status,"hook=on-hook",12)){
            LMLOG(LINF, "%s: The linphone is on-hook.", __FUNCTION__);
            return ON_HOOK;
        }

        if(0 == memcmp(status,"hook=ringing",12)){
            LMLOG(LINF, "%s: The linphone is ringing.", __FUNCTION__);
            return RINGING;
        }

        if(0 == memcmp(status,"Call",4)){
            LMLOG(LINF, "%s: The linphone is Call.", __FUNCTION__);
            return CALL;
        }
        else{
            return BAD;
        }
    }

    pclose(fp);

    return BAD;
}

int calls_status(void)
{
    //int ret;
    //ret = get_status();

    if(LINPHONE_STATUS == CALL){
        LMLOG(LWRN, "%s: The linphone is calling.", __FUNCTION__);
        return BAD;
    }else{
        LMLOG(LINF, "%s: The linphone is on_hook.", __FUNCTION__);
        return GOOD;
    }
}

int terminal_calls(void)
{
    return GOOD;
}

void process_button_event(void)
{
    int status = 0;
    int ret;
    int cnt = 3;
    int j = 40000000;

    //judgement is knob down or release button
    if(event == DOWN){
        //judgement the linphone calls is running or not
        status = calls_status();
        if(status != GOOD){
            LMLOG(LWRN, "%s: The linphone is Calling or Ringning, the status is %d",  __FUNCTION__, status);
            return;
        }

        while(j){
            j--;
            if(j % 5000000 == 0){
                LMLOG(LINF, "%s: The J value is %d ", __FUNCTION__, j );
            }
        }

        if(key_array[0] != KEY_GPIO9_4_DOWN && event != DOWN){
            LMLOG(LWRN, "%s: Touch button by mistake.", __FUNCTION__);
            return;
        }

        recorder_setup();
        pthread_create(&recorder->thread_id, NULL, (void *)snd_record_start, NULL);
    }

    if(event == UP){
        if(recorder != NULL){

            uint8_t packet[24] = {0};
            int packet_len = 24;
            IPinterCom_control_hdr *ip_hdr = (IPinterCom_control_hdr *)packet;
            ip_addr_t dest_ip;
            int remote_port = global_cfg.dest_port;

            snd_record_stop();

            while(recorder->state != STOPPING){
                LMLOG(LINF, "%s: Waiting the recorder stop...", __FUNCTION__);
                snd_record_stop();
                usleep(100000);
            }

            if(recorder->aud_raw_fp != NULL){
                LMLOG(LINF, "%s: Close the stream file fp.", __FUNCTION__);
                fclose(recorder->aud_raw_fp);
                recorder->aud_raw_fp = NULL;
                ret = snd_ftp_put(recorder->record_aud_file);
                if(ret != GOOD){
                    LMLOG(LERR, "%s:Send aud file failed ret = %d.", __FUNCTION__,ret);
                    return;
                }

                LMLOG(LINF, "%s: Finish the file transport.", __FUNCTION__);

                ip_hdr->src  = global_cfg.dev_num;
                ip_hdr->dest = global_cfg.host_num;
                ip_hdr->cmd = PUT_CMD;
                memcpy(ip_hdr->filename, recorder->record_aud_file,FILENAME_LEN);
                memset(recorder->record_aud_file, 0, FILENAME_LEN);
            }

            recorder_destroy();

            dest_ip.afi  = AF_INET;
            dest_ip.addr.v4.s_addr =  inet_addr(global_cfg.host_ip);

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

    return;
}

void signal_f(int signum)
{
    unsigned int i;
    int ret = 0;

    ret = read(fd, &key_array, sizeof(key_array));
    if(ret < 0){
        LMLOG(LERR, "%s: First Read the key value error, the return error is %d",__FUNCTION__, ret);
    }

    for(i=0; i<KEY_NUM; i++){
        LMLOG(LINF, "The Ket value is %02x ", key_array[i]);
    }

    if(key_array[0] == KEY_GPIO9_4_DOWN && event == UP){
        event = DOWN;
        LMLOG(LDBG_1, "%s: The button event is %d", __FUNCTION__, event);
        process_button_event();
    }

    if(key_array[0] == KEY_GPIO9_4_UP && event == DOWN){
        event = UP;
        LMLOG(LDBG_1, "%s: The button event is %d", __FUNCTION__, event);
        process_button_event();
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
