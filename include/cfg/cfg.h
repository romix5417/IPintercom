#ifndef __CFG_H_
#define __CFG_H_

typedef struct Cfg_Opt{
    char host_ip[16];
    int  host_num;
    int  dev_num;
    int  debug_level;
    int  dest_port;
    int  local_port;
    int  ftp_port;
}CFG_OPT;


int cfg_read(char *cfg_name, CFG_OPT * global_cfg);

#endif
