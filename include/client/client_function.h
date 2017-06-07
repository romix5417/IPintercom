#ifndef __CLIENT_FUNCTION_H__
#define __CLIENT_FUNCTION_H__

#include "socket/ipintercom_sockets.h"

int process_ctl_msg(int sock, int afi, struct in_addr client_ip);
int process_get_cmd(int sock, uint8_t *packet, ip_addr_t dest_ip, uint16_t remote_port);
int check_file_exist(char *name);

#endif
