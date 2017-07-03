/*
 *
 * Copyright (C) 2011, 2015 Cisco Systems, Inc.
 * Copyright (C) 2015 CBA research group, Technical University of Catalonia.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <inttypes.h>
#include <netdb.h>
#include <net/if.h>
#include <netinet/in_systm.h>
//#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/ioctl.h>
#include <syslog.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <endian.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <ifaddrs.h>
//#include <bits/in.h>

#include "socket/sockets-util.h"
#include "log/lmlog.h"

/*struct in_pktinfo {
	int		ipi_ifindex;
	struct in_addr	ipi_spec_dst;
	struct in_addr	ipi_addr;
};*/

int open_ip_raw_socket(int afi)
{
    int s;
    int on = 1;

    if ((s = socket(afi, SOCK_RAW, IPPROTO_RAW)) < 0) {
        LMLOG(LERR, "open_ip_raw_socket: socket creation failed"
                " %s", strerror(errno));
        return (ERR_SOCKET);
    }

    if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int)) == -1) {
        LMLOG(LWRN, "open_ip_raw_socket: socket option reuse %s",
                strerror(errno));
        close(s);
        return (ERR_SOCKET);
    }

    LMLOG(LDBG_3, "open_ip_raw_socket: open socket %d with afi: %d", s, afi);

    return s;

}

int open_udp_raw_socket(int afi)
{
    struct protoent *proto = NULL;
    int sock = ERR_SOCKET;
    int tr = 1;
    int protonum = 0;

#ifdef ANDROID
    protonum = IPPROTO_UDP;
#else
    if ((proto = getprotobyname("UDP")) == NULL) {
        LMLOG(LERR, "open_udp_raw_socket: getprotobyname: %s", strerror(errno));
        return(-1);
    }
    protonum = proto->p_proto;
#endif

    /*
     *  build the ipv4_data_input_fd, and make the port reusable
     */

    if ((sock = socket(afi, SOCK_RAW, protonum)) < 0) {
        LMLOG(LERR, "open_udp_raw_socket: socket: %s", strerror(errno));
        return (ERR_SOCKET);
    }
    LMLOG(LDBG_3, "open_udp_raw_socket: Created socket %d associated to %s addresses\n",
            sock, (afi == AF_INET) ? "IPv4":"IPv6");

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &tr, sizeof(int)) == -1) {
        LMLOG(LWRN,"open_udp_raw_socket: setsockopt SO_REUSEADDR: %s",
                strerror(errno));
        close(sock);
        return (ERR_SOCKET);
    }

    return (sock);
}

int open_udp_datagram_socket(int afi)
{
    struct protoent *proto = NULL;
    int sock = ERR_SOCKET;
    int tr = 1;
    int protonum = 0;

#ifdef ANDROID
    protonum = IPPROTO_UDP;
#else
    if ((proto = getprotobyname("UDP")) == NULL) {
        LMLOG(LERR, "open_udp_datagram_socket: getprotobyname: %s", strerror(errno));
        return(ERR_SOCKET);
    }
    protonum = proto->p_proto;
#endif

    if ((sock = socket(afi, SOCK_DGRAM, protonum)) < 0) {
        LMLOG(LERR, "open_udp_datagram_socket: socket: %s", strerror(errno));
        return (ERR_SOCKET);
    }
    LMLOG(LDBG_3, "open_udp_datagram_socket: Created socket %d associated to %s addresses\n",
            sock, (afi == AF_INET) ? "IPv4":"IPv6");

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &tr, sizeof(int)) == -1) {
        LMLOG(LWRN, "open_udp_datagram_socket: setsockopt SO_REUSEADDR: %s",
                strerror(errno));

        return (ERR_SOCKET);
    }

    return sock;
}

/* XXX: binding might not work on all devices */
#if 0
inline int
socket_bindtodevice(int sock, char *device)
{
    int device_len = 0;

    device_len = strlen(device);
    if (setsockopt(sock, SOL_SOCKET, SO_BINDTODEVICE, device, device_len) == -1) {
        LMLOG(LWRN, "socket_bindtodevice: Error binding socket to device %s:",
                strerror(errno));
        return (BAD);
    }
    return (GOOD);
}

inline int
socket_conf_req_ttl_tos(int sock, int afi)
{
    const int on = 1;

    switch (afi) {
    case AF_INET:

        /* IP_RECVTOS is requiered to get later the IPv4 original TOS */
        if (setsockopt(sock, IPPROTO_IP, IP_RECVTOS, &on, sizeof(on)) < 0) {
            LMLOG(LWRN, "open_data_raw_input_socket: setsockopt IP_RECVTOS: %s", strerror(errno));
            return (BAD);
        }

        /* IP_RECVTTL is requiered to get later the IPv4 original TTL */
        if (setsockopt(sock, IPPROTO_IP, IP_RECVTTL, &on, sizeof(on)) < 0) {
            LMLOG(LWRN, "open_data_raw_input_socket: setsockopt IP_RECVTTL: %s", strerror(errno));
            return (BAD);
        }

        break;

    case AF_INET6:

        /* IPV6_RECVTCLASS is requiered to get later the IPv6 original TOS */
        if (setsockopt(sock, IPPROTO_IPV6, IPV6_RECVTCLASS, &on, sizeof(on))
                < 0) {
            LMLOG(LWRN, "open_data_raw_input_socket: setsockopt IPV6_RECVTCLASS: %s", strerror(errno));
            return (BAD);
        }

        /* IPV6_RECVHOPLIMIT is requiered to get later the IPv6 original TTL */
        if (setsockopt(sock, IPPROTO_IPV6, IPV6_RECVHOPLIMIT, &on, sizeof(on))
                < 0) {
            LMLOG(LWRN, "open_data_raw_input_socket: setsockopt IPV6_RECVHOPLIMIT: %s", strerror(errno));
            return (BAD);
        }

        break;

    default:
        return (BAD);
    }

    return (GOOD);
}
#endif

/*
 * other ip functions
 */

char *
ip_to_char(void *ip, int afi)
{
    static char address[10][INET6_ADDRSTRLEN+1];
    static unsigned int i;
    i++; i = i % 10;
    *address[i] = '\0';
    switch (afi) {
    case AF_INET:
        inet_ntop(AF_INET, ip, address[i], INET_ADDRSTRLEN);
        return(address[i]);
    case AF_INET6:
        inet_ntop(AF_INET6, ip, address[i], INET6_ADDRSTRLEN);
        return(address[i]);
    }

    return(NULL);
}


char *
ip_addr_to_char(ip_addr_t *addr)
{
    return(ip_to_char(ip_addr_get_addr(addr), get_addr_afi(addr)));
}


/*
 * Bind a socket to a specific address and port if specified
 * Afi is used when the src address is not specified
 */
int
bind_socket(int sock, int afi, ip_addr_t *src_addr, int src_port)
{
    int result = TRUE;
    struct sockaddr *sock_addr;
    int sock_addr_len;
    struct sockaddr_in sock_addr_v4;
    struct sockaddr_in6 sock_addr_v6;

    switch(afi){
    case AF_INET:
        memset ( ( char * ) &sock_addr_v4, 0, sizeof ( sock_addr_v4 ) );
        sock_addr_v4.sin_family = AF_INET;
        if (src_port != 0){
            sock_addr_v4.sin_port        = htons(src_port);
        }
        if (src_addr != NULL){
            sock_addr_v4.sin_addr.s_addr = ip_addr_get_v4(src_addr)->s_addr;
        }else{
            sock_addr_v4.sin_addr.s_addr = INADDR_ANY;
        }

        sock_addr = ( struct sockaddr * ) &sock_addr_v4;
        sock_addr_len = sizeof ( struct sockaddr_in );

        break;
    case AF_INET6:
        memset ( ( char * ) &sock_addr_v6, 0, sizeof ( sock_addr_v6 ) );
        sock_addr_v6.sin6_family = AF_INET6;
        if (src_port != 0){
            sock_addr_v6.sin6_port     = htons(src_port);
        }
        if (src_addr != NULL){
            memcpy(&(sock_addr_v6.sin6_addr),ip_addr_get_v6(src_addr),sizeof(struct in6_addr));
        }else{
            sock_addr_v6.sin6_addr     = in6addr_any;
        }

        sock_addr = ( struct sockaddr * ) &sock_addr_v6;
        sock_addr_len = sizeof ( struct sockaddr_in6 );

        break;
    default:
        return (BAD);
    }

    if (bind(sock,sock_addr,sock_addr_len) != 0){
        LMLOG(LDBG_1, "bind_socket: %s", strerror(errno));
        result = BAD;
    }else{
        LMLOG(LDBG_1, "bind_socket: Binded socket %d to source address %s and port %d",
                sock, ip_addr_to_char(src_addr),src_port);
    }

    return (result);
}


/* Sends a raw packet out the socket file descriptor 'sfd'  */
int
send_raw_packet(int socket, const void *pkt, int plen, ip_addr_t *dip)
{
    struct sockaddr *saddr = NULL;
    int slen, nbytes;

    struct sockaddr_in sa4;
    struct sockaddr_in6 sa6;

    /* build sock addr */
    switch (get_addr_afi(dip)) {
    case AF_INET:
        memset(&sa4, 0, sizeof(sa4));
        sa4.sin_family = AF_INET;
        ip_addr_copy_to(&sa4.sin_addr, dip);
        slen = sizeof(struct sockaddr_in);
        saddr = (struct sockaddr *)&sa4;
        break;
    case AF_INET6:
        memset(&sa6, 0, sizeof(sa6));
        sa6.sin6_family = AF_INET6;
        ip_addr_copy_to(&sa6.sin6_addr, dip);
        slen = sizeof(struct sockaddr_in6);
        saddr = (struct sockaddr *)&sa6;
        break;
    }

    nbytes = sendto(socket, pkt, plen, 0, saddr, slen);
    if (nbytes != plen) {
        LMLOG(LDBG_2, "send_raw_packet: send packet to %s using fail descriptor %d failed -> %s", ip_addr_to_char(dip),
                socket, strerror(errno));
        return(BAD);
    }

    return (GOOD);
}

int
send_datagram_packet (int sock, const void *packet, int packet_length,
        ip_addr_t *addr_dest, int port_dest)
{
    struct sockaddr_in sock_addr_v4;
    struct sockaddr_in6 sock_addr_v6;
    struct sockaddr *sock_addr = NULL;
    int sock_addr_len = 0;

    switch (get_addr_afi(addr_dest)){
    case AF_INET:
        memset(&sock_addr_v4,0,sizeof(sock_addr_v4));           /* be sure */
        sock_addr_v4.sin_port        = htons(port_dest);
        sock_addr_v4.sin_family      = AF_INET;
        sock_addr_v4.sin_addr.s_addr = ip_addr_get_v4(addr_dest)->s_addr;
        sock_addr = (struct sockaddr *) &sock_addr_v4;
        sock_addr_len = sizeof(sock_addr_v4);
        break;
    case AF_INET6:
        memset(&sock_addr_v6,0,sizeof(sock_addr_v6));                   /* be sure */
        sock_addr_v6.sin6_family   = AF_INET6;
        sock_addr_v6.sin6_port     = htons(port_dest);
        memcpy(&sock_addr_v6.sin6_addr, ip_addr_get_v6(addr_dest),sizeof(struct in6_addr));
        sock_addr = (struct sockaddr *) &sock_addr_v6;
        sock_addr_len = sizeof(sock_addr_v6);
        break;
    default:
        LMLOG(LDBG_2, "send_datagram_packet: Unknown afi %d",get_addr_afi(addr_dest));
        return (BAD);
    }

    if (sendto(sock, packet, packet_length, 0, sock_addr, sock_addr_len) < 0){
        LMLOG(LDBG_2, "send_datagram_packet: send failed %s.",strerror ( errno ));
        return (BAD);
    }
    return (GOOD);
}

/*
 * Get a packet from the socket. It also returns the destination addres and source port of the packet
 */

int get_packet_and_socket_inf (
        int             sock,
        int             afi,
        uint8_t         *packet,
        ip_addr_t       *remote_ip,
        uint16_t        *remote_port)
{
    union control_data {
        struct cmsghdr cmsg;
        unsigned char data4[CMSG_SPACE(sizeof(struct in_pktinfo))]; /* Space for IPv4 pktinfo */
        unsigned char data6[CMSG_SPACE(sizeof(struct in6_pktinfo))]; /* Space for IPv6 pktinfo */
    };

    struct sockaddr_in  s4;
    struct sockaddr_in6 s6;
    struct msghdr       msg;
    struct iovec        iov[1];
    union control_data  cmsg;
    struct cmsghdr      *cmsgptr    = NULL;
    int                 nbytes      = 0;

    iov[0].iov_base = packet;
    iov[0].iov_len = MAX_IP_PACKET;

    memset(&msg, 0, sizeof msg);
    msg.msg_iov = iov;
    msg.msg_iovlen = 1;
    msg.msg_control = &cmsg;
    msg.msg_controllen = sizeof cmsg;

    if (afi == AF_INET){
        msg.msg_name = &s4;
        msg.msg_namelen = sizeof (struct sockaddr_in);
    }else{
        msg.msg_name = &s6;
        msg.msg_namelen = sizeof (struct sockaddr_in6);
    }

    nbytes = recvmsg(sock, &msg, 0);
    if (nbytes == -1) {
        LMLOG(LERR, "read_packet: recvmsg error: %s", strerror(errno));
        return (BAD);
    }

    if (afi == AF_INET){
        for (cmsgptr = CMSG_FIRSTHDR(&msg); cmsgptr != NULL; cmsgptr = CMSG_NXTHDR(&msg, cmsgptr)) {
            if (cmsgptr->cmsg_level == IPPROTO_IP && cmsgptr->cmsg_type == IP_PKTINFO) {
                remote_ip->afi = AF_INET;
                remote_ip->addr.v4 = ((struct in_pktinfo *)(CMSG_DATA(cmsgptr)))->ipi_addr;
                LMLOG(LINF, "read_packet: recvmsg ip addr: 0x%x", remote_ip->addr.v4.s_addr);
                break;
            }
        }

        *remote_port = ntohs(s4.sin_port);
    }else {
	    LMLOG(LINF, "read_packet: recvmsg IPv6 addr");
        for (cmsgptr = CMSG_FIRSTHDR(&msg); cmsgptr != NULL; cmsgptr = CMSG_NXTHDR(&msg, cmsgptr)) {
            if (cmsgptr->cmsg_level == IPPROTO_IPV6 && cmsgptr->cmsg_type == IPV6_PKTINFO) {
                remote_ip->afi = AF_INET6;
                memcpy(&(remote_ip->addr.v6.s6_addr),
                        &(((struct in6_pktinfo *)(CMSG_DATA(cmsgptr)))->ipi6_addr.s6_addr),
                        sizeof(struct in6_addr));
                break;
            }
        }
        *remote_port = ntohs(s6.sin6_port);
    }

    return (GOOD);
}

struct in_addr *
ip_addr_get_v4(ip_addr_t *ipaddr)
{
    return(&(ipaddr->addr.v4));
}

struct in6_addr *
ip_addr_get_v6(ip_addr_t *ipaddr)
{
    return(&(ipaddr->addr.v6));
}

int get_addr_afi(ip_addr_t *ipaddr)
{
    return ipaddr->afi;
}

void *
ip_addr_get_addr(ip_addr_t *ipaddr)
{
    return (&(ipaddr->addr));
}

uint8_t
ip_sock_afi_to_size(uint16_t afi)
{
    switch (afi) {
    case AF_INET:
        return(sizeof(struct in_addr));
    case AF_INET6:
        return(sizeof(struct in6_addr));
    default:
        LMLOG(LWRN, "ip_sock_afi_to_size: unknown IP AFI (%d)", afi);
        return(0);
    }
}

uint8_t
ip_addr_get_size(ip_addr_t *ipaddr)
{
    return(ip_sock_afi_to_size(get_addr_afi(ipaddr)));
}


/* ip_addr_copy_to
 *
 * @dst : memory location
 * @src : the ip address to be copied
 * Description: The function copies what is *CONTAINED* in an ip address
 * to a given memory location, NOT the whole structure! See ip_addr_copy
 * for copying ip addresses
 */
void
ip_addr_copy_to(void *dst, ip_addr_t *src)
{
    if (!dst || !src) {
        return;
    }
    memcpy(dst, ip_addr_get_addr(src), ip_addr_get_size(src));
}

int send_cmd_packet(uint8_t *packet, int packet_len, ip_addr_t dest_ip,int remote_port)
{
    int socket_fd;
    struct sockaddr_in dst_address;
    int ret = 0;

    // Create a TCP socket
    socket_fd=socket(AF_INET,SOCK_STREAM,0);//IPV4  SOCK_STREAM (TCP protocol)

    bzero(&dst_address,sizeof(dst_address));
    dst_address.sin_family=dest_ip.afi;
    dst_address.sin_addr.s_addr = dest_ip.addr.v4.s_addr;
    dst_address.sin_port=htons(remote_port);

    ret = connect(socket_fd, (struct sockaddr *)&dst_address, sizeof(struct sockaddr));
    if(ret != 0){
        LMLOG(LERR, "%s: Connection failed,the Error is %d.", __FUNCTION__ , ret);
        close(socket_fd);

        return BAD;
    }

    ret = send(socket_fd, packet, packet_len, 0);
    if(ret != packet_len){
        LMLOG(LERR, "%s: Send failed, the Error is %d.", __FUNCTION__ , ret);
        close(socket_fd);

        return BAD;
    }

    close(socket_fd);

    return (EXIT_SUCCESS);
}
