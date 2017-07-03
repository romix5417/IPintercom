#ifndef __CLIENT_FUNCTION_H__
#define __CLIENT_FUNCTION_H__

#include "socket/ipintercom_sockets.h"

typedef struct IPinterCom_control_hdr{
    uint8_t src;
    uint8_t dest;
    uint8_t cmd;
    uint8_t Reserve;
    int  filesize;
    char filename[16];
}IPinterCom_control_hdr;

int process_ctl_msg(int sock, int afi, struct in_addr client_ip);
int process_get_cmd(int sock, uint8_t *packet, ip_addr_t dest_ip, uint16_t remote_port);
int check_file_exist(char *name);

#define PUT_CMD  0
#define GET_CMD  1
#define DIAL_CMD 2
#define STOP_CMD 3

#endif
