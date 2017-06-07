/*
 * File:   msg_test.c
 * Author: luomin
 */
#include <stdio.h>
#include <stdlib.h>

#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <errno.h>

#define TCP_MODE

/*status msg*/
unsigned char test_msg[]={
		                  0x00,0x00,0x01,0x03,
		                  0x00,0x00,0x00,0x00,
			              0x00,0x00,0x00,0x00,
			              0x00,0x00,0x00,0x00,
				          0x10,0x00,0x00,0x00,
		                  0x00,0x00,0x00,0x00,
				          0x20,0x00,0x00,0x00,
			              0x00,0x00,0x00,0x00,
						  0x00,0x00,0x00,0x00,
						  0x00,0x00,0x00,0x00,
                         };

/*video cmd msg*/
unsigned char test_video_cmd_msg[]={
		                  0x10,0x00,0x01,0x01,
		                  0x00,0x00,0x00,0x00,
			              0x00,0x19,0x01,0x40,
			              0x00,0xf0,0x00,0x00,
                         };

/*audio cmd msg*/
unsigned char test_audio_cmd_msg[]={
		                  0x00,0x01,0x01,0x00,
		                  0x00,0x00,0x00,0x00,
			              0x00,0x00,0x00,0x00,
                          0x00,0x00,0x00,0x00,
                          0x00,0x00,0x00,0x00,
                          0x00,0x00,0x00,0x00,
                          0x00,0x00,0x00,0x00,
                         };

typedef struct msg_header{
    uint8_t  src;
    uint8_t  dest;
    uint8_t  cmd;
    uint8_t  reserve;
    uint32_t filesize;
    uint8_t  name[16];
}msg_header;

#ifdef UDP_MODE
int port=9999;
int main(int argc, char** argv) {
    int socket_descriptor; //套接口描述字
    int iter=0;
    char buf[80];
    struct sockaddr_in address;//处理网络通信的地址
    char aud_name[16]={"0220170607150830"};
    msg_header *msg;

    msg = (msg_header *)test_audio_cmd_msg;
    memcpy(msg->name,aud_name,sizeof(aud_name));

    bzero(&address,sizeof(address));
    address.sin_family=AF_INET;
    address.sin_addr.s_addr=inet_addr("127.0.0.1");//这里不一样
    address.sin_port=htons(port);

    //创建一个 UDP socket

    socket_descriptor=socket(AF_INET,SOCK_DGRAM,0);//IPV4  SOCK_DGRAM 数据报套接字（UDP协议）

    //for(iter=0;iter<=20;iter++)
    //{
         /*
         * sprintf(s, "%8d%8d", 123, 4567); //产生：" 123 4567"
         * 将格式化后到 字符串存放到s当中
         */
        //sprintf(buf,"data packet with ID %d\n",iter);

        /*int PASCAL FAR sendto( SOCKET s, const char FAR* buf, int len, int flags,const struct sockaddr FAR* to, int tolen);
         * s：一个标识套接口的描述字。
         * buf：包含待发送数据的缓冲区。
         * len：buf缓冲区中数据的长度。
         * flags：调用方式标志位。
         * to：（可选）指针，指向目的套接口的地址。
         * tolen：to所指地址的长度。
　　      */
        //sendto(socket_descriptor,buf,sizeof(buf),0,(struct sockaddr *)&address,sizeof(address));
    //}
	sendto(socket_descriptor,test_video_cmd_msg,sizeof(test_video_cmd_msg),0,(struct sockaddr *)&address,sizeof(address));

    sprintf(buf,"stop\n");
    sendto(socket_descriptor,buf,sizeof(buf),0,(struct sockaddr *)&address,sizeof(address));//发送stop 命令
    close(socket_descriptor);
    printf("Messages Sent,terminating\n");

    exit(0);

    return (EXIT_SUCCESS);
}

#else
int dst_port=9990;
int my_port=9999;

int main(int argc, char** argv) {
    int socket_descriptor; //套接口描述字
    int iter=0;
    char buf[80];
    struct sockaddr_in address,dst_address;//处理网络通信的地址

    char aud_name[16]={"0220170607150830"};
    msg_header *msg;

    msg = (msg_header *)test_audio_cmd_msg;
    memcpy(msg->name, aud_name, sizeof(aud_name));

    bzero(&address,sizeof(address));
    address.sin_family=AF_INET;
    address.sin_addr.s_addr=htonl(INADDR_ANY);//这里不一样
    address.sin_port=htons(my_port);

    //创建一个 UDP socket
    socket_descriptor=socket(AF_INET,SOCK_STREAM,0);//IPV4  SOCK_DGRAM 数据报套接字（UDP协议）
    if(bind(socket_descriptor,(struct sockaddr *)&address,sizeof(struct sockaddr))==-1){
        printf("Bind error...%d\r\n",errno);
        close(socket_descriptor);
        exit(1);
    }

    bzero(&dst_address,sizeof(dst_address));
    dst_address.sin_family=AF_INET;
    dst_address.sin_addr.s_addr=inet_addr("192.168.10.100");//这里不一样
    dst_address.sin_port=htons(dst_port);

    if(connect(socket_descriptor, (struct sockaddr *)&dst_address, sizeof(struct sockaddr)) == -1)
    {
        printf("Connection Error, %d\n", errno);
        close(socket_descriptor);
        exit(1);
    }

    //for(iter=0;iter<=20;iter++)
    //{
         /*
         * sprintf(s, "%8d%8d", 123, 4567); //产生：" 123 4567"
         * 将格式化后到 字符串存放到s当中
         */
        //sprintf(buf,"data packet with ID %d\n",iter);

        /*int PASCAL FAR sendto( SOCKET s, const char FAR* buf, int len, int flags,const struct sockaddr FAR* to, int tolen);
         * s：一个标识套接口的描述字。
         * buf：包含待发送数据的缓冲区。
         * len：buf缓冲区中数据的长度。
         * flags：调用方式标志位。
         * to：（可选）指针，指向目的套接口的地址。
         * tolen：to所指地址的长度。
　　      */
        //sendto(socket_descriptor,buf,sizeof(buf),0,(struct sockaddr *)&address,sizeof(address));
    //}
    //

    printf("%s: msg aud file name is %s.\n", __FUNCTION__, msg->name);
    sendto(socket_descriptor,(char *)msg,sizeof(struct msg_header),0,(struct sockaddr *)&address,sizeof(address));

    //sprintf(buf,"stop\n");
    //sendto(socket_descriptor,buf,sizeof(buf),0,(struct sockaddr *)&address,sizeof(address));//发送stop 命令
    close(socket_descriptor);
    printf("Messages Sent,terminating\n");

    exit(0);

    return (EXIT_SUCCESS);
}
#endif
