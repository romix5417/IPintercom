#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "socket/sockets-util.h"
#include "log/lmlog.h"
#include "client/client_function.h"
#include "client/button.h"
#include "transport/transport_function.h"
#include "defs.h"
#include "audio/aud.h"
#include "message/message.h"
#include "cfg/cfg.h"


#define FILENAME_LEN 16

extern CFG_OPT global_cfg;

extern Recorder *recorder;
extern Player *player;

int check_file_exist(char *name)
{
    LMLOG(LINF, "%s: The %s file is exist.", __FUNCTION__, name);

    return GOOD;
}

int process_get_cmd(int sock, uint8_t *packet, ip_addr_t dest_ip, uint16_t remote_port)
{
    char name[32]={0};
    FILE *aud_encode_fp, *aud_raw_fp;
    int status = 0;
    int ret = 0;
    char buf[32] = {0};
    char decode_file_name[32]={0};//{"raw_"};
    VoiceMessage *vmsg = NULL;

    memcpy(name, (char *)(((IPinterCom_control_hdr *)packet)->filename), FILENAME_LEN);
    LMLOG(LINF, "%s: The aud file name is %s.", __FUNCTION__, name);

    snd_ftp_get(name);
    ret = check_file_exist(name);
    if(ret!=GOOD){
        LMLOG(LERR, "%s:The %s file not exist!", __FUNCTION__, name);

        return ret;
    }

    vmsg = (VoiceMessage *)malloc(sizeof(VoiceMessage));
    LMLOG(LINF, "%s: The VoiceMessage alloc success.", __FUNCTION__);

    player_setup();

    memcpy(vmsg->filename, name,FILENAME_LEN);
    LMLOG(LINF, "%s: The vmsg filename %s, vmsg address:%x, vmsg filename address:%x.", __FUNCTION__, vmsg->filename, vmsg, vmsg->filename);
    fifo_enqueue(player->msg_fifo,&vmsg);

    //judgement the linphone calls is running or not
    status = calls_status();
    if(status != GOOD){
        LMLOG(LWRN, "%s: The linphone is Calling or Ringning, the status is %d",  __FUNCTION__, status);
        return status;
    }

    if(player->play_flag != RUNNING){
        player->play_flag = RUNNING;
        snd_play_start(player->sndCard);
    }

    return GOOD;
}

void linphone_dail(void)
{
    int buf[64]={0};

    if(recorder != NULL){
        if(recorder->state == RUNNING){
            LMLOG(LWRN, "%s: The recorder is running.", __FUNCTION__);
            return;
        }
    }

    if(player != NULL){
        if(player->play_flag == RUNNING){
            LMLOG(LWRN, "%s: The player is running.", __FUNCTION__);
            return;
        }
    }

    sprintf(buf, "linphonecsh dial sip:root@%s &", global_cfg.host_ip);
    LMLOG(LINF, "%s: The dial cmd is '%s'.", __FUNCTION__, buf);
    system(buf);
    system("echo 'dial run' > log");

    return;
}

/*
 *  Process a IPinterCom protocol message sitting on
 *  socket s with address family afi
 */
int process_ctl_msg(int sock, int afi, struct in_addr client_ip)
{
    uint8_t     packet[MAX_IP_PACKET];
    uint16_t    remote_port;
    ip_addr_t   dest_ip;
    int i;

    memset(packet,0,sizeof(packet));
    LMLOG(LINF, "%s: The packet size is %d.", __FUNCTION__, sizeof(packet));

    if ( get_packet_and_socket_inf (sock, afi, packet,&dest_ip,&remote_port) != GOOD ){
        return BAD;
    }

    dest_ip.addr.v4.s_addr = client_ip.s_addr;
    LMLOG(LINF, "%s: Received a IPinterCom control message,the dest_ip:0x%x,remote_port:%d",__FUNCTION__, dest_ip.addr.v4.s_addr,remote_port);
    LMLOG(LINF, "%s: Received msg:\n", __FUNCTION__);

    for(i = 0; i < sizeof(packet); i++){
        if(i%16==0 && i != 0){
            printf("\n");
        }
        printf("%3x",packet[i]);
    }
    printf("\r\n");

    switch(((IPinterCom_control_hdr *)packet)->cmd){
        case GET_CMD:    //Got Cmd Setting Reply
            LMLOG(LINF, "Received a IPinterCom Cmd message");
            if (process_get_cmd(sock,packet,dest_ip,remote_port) != GOOD){
                return (BAD);
            }
            break;
        case DIAL_CMD:
            linphone_dail();
            break;
        default:
            LMLOG(LINF, "Unidentified type control message received");
            return (ERROR);
    }

    LMLOG(LINF, "Processing Completed to IPinterCom protocol control message");

    return(GOOD);
}
