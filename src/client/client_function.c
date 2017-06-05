#include <socket/souckets-util.h>
#include <log/lmlog.h>

typedef struct IPinterCom_control_hdr{
    uint8_t src;
    uint8_t dest;
    uint8_t cmd;
    uint8_t Reserve;
    int  filesize;
    char filename[16];
}IPinterCom_control_hdr;

int process_cmd_msg(int sock, uint8_t *packet, ip_addr_t dest_ip, uint16_t remote_port)
{
    char *name, *decode_file_name;
    FILE *aud_encode_fp, *aud_raw_fp;
    int status = 0;

    name = ((IPinterCom_control_hdr *)packet)->filename;

    snd_tftp_get(name);
    ret = check_file_exist(name);
    if(ret!=Good){
        LMLOG(LERR, "%s:The %s file not exist!", __FUNCTION__, name);

        return ret;
    }

    sprintf(decode_file_name, "raw_%s", name);

    aud_encode_fp = fopen(name, 'r');
    if(aud_encode_fp == NULL){
        LMLOG(LERR, "%s:The %s file not exist!", __FUNCTION__, name);

        return BAD;
    }

    aud_raw_fp = fopen(decode_file_name, 'w+');
    if(aud_raw_fp == NULL){
        LMLOG(LERR, "%s:The %s file open faied!", __FUNCTION__, name);

        return BAD;
    }

    snd_decode_start(aud_encode_fp, aud_raw_fp);

    fclose(aud_encode_fp);

    //judgement the linphone calls is running or not
    status = calls_status();
    if(status == RUNNING){
        fclose(aud_raw_fp);
        return BAD;
    }

    snd_start_play

    reader->snd_read(card, recorder->aud_raw_fp,recorder->sample);

    return Good;
}

/*
 *  Process a IPinterCom protocol message sitting on
 *  socket s with address family afi
 */
int process_ctl_msg(
        int sock,
        int afi,
	struct in_addr client_ip)
{
    uint8_t     packet[MAX_IP_PACKET];
    uint16_t    remote_port;
    ip_addr_t   dest_ip;

    if ( get_packet_and_socket_inf (sock, afi, packet,&dest_ip,&remote_port) != GOOD ){
        return BAD;
    }

    dest_ip.addr.v4.s_addr = client_ip.s_addr;
    LMLOG(LINF, "Received a IPinterCom control message,the dest_ip:0x%x,remote_port:%d",dest_ip.addr.v4.s_addr,remote_port);

    switch (((IPinterCom_control_hdr_t *) packet)->cmd) {
        case MESHCOM_CTL_CMD:    //Got Cmd Setting Reply
            LMLOG(LINF, "Received a IPinterCom Cmd message");
            if (process_cmd_msg(sock,packet,dest_ip,remote_port) != GOOD){
                return (BAD);
            }
            break;
        default:
            LMLOG(LINF, "Unidentified type control message received");
            return (Error);
    }

    LMLOG(LINF, "Processing Completed to IPinterCom protocol control message");

    return(GOOD);
}
