#ifndef  __IPINTERCOM_SOCKETS_H
#define  __IPINTERCOM_SOCKETS_H

#include <arpa/inet.h>
#include <net/if.h>
//#include <netinet/in.h>
#include <netinet/in_systm.h>
#include <netinet/ip6.h>

#include "defs.h"

/*
 * IP address type
 */
typedef struct {
    int      afi;
    union {
        struct in_addr      v4;
        struct in6_addr     v6;
    } addr;
}ip_addr_t;

#endif
