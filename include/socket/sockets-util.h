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

#ifndef SOCKETS_UTIL_H_
#define SOCKETS_UTIL_H_

#include <string.h>

#include "ipintercom_sockets.h"
#include "log/lmlog.h"

int open_ip_raw_socket(int afi);
int open_udp_raw_socket(int afi);

int open_udp_datagram_socket(int afi);
//inline int socket_bindtodevice(int sock, char *device);
//inline int socket_conf_req_ttl_tos(int sock, int afi);

int bind_socket(int sock,int afi, ip_addr_t *src_addr, int src_port);
int send_raw_packet(int, const void *, int, ip_addr_t *);
int send_datagram_packet (int sock, const void *packet, int packet_length,
        ip_addr_t *addr_dest, int port_dest);

/*
 * Get a packet from the socket. It also returns the destination addres and source port of the packet.
 * Used for control packets
 */

int get_packet_and_socket_inf (
        int             sock,
        int             afi,
        uint8_t         *packet,
        ip_addr_t       *remote_ip,
        uint16_t        *remote_port);


struct in_addr *
ip_addr_get_v4(ip_addr_t *ipaddr);

struct in6_addr *
ip_addr_get_v6(ip_addr_t *ipaddr);

int get_addr_afi(ip_addr_t *ipaddr);

void *
ip_addr_get_addr(ip_addr_t *ipaddr);

uint8_t
ip_sock_afi_to_size(uint16_t afi);

uint8_t
ip_addr_get_size(ip_addr_t *ipaddr);

void
ip_addr_copy_to(void *dst, ip_addr_t *src);

#endif /* SOCKETS_UTIL_H_ */
