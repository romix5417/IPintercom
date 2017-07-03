#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <netinet/in.h>

const int port = 4100;
const char* ip = "192.168.10.101";

int main()
{
      //创建套接字,即创建socket
      int ser_sock = socket(AF_INET, SOCK_STREAM, 0);
      if(ser_sock < 0)
      {
          //创建失败
          perror("socket");
          return 1;
      }

      //绑定信息，即命名socket
      struct sockaddr_in addr;
      addr.sin_family = AF_INET;
      addr.sin_port = htons(port);
      //inet_addr函数将用点分十进制字符串表示的IPv4地址转化为用网络
      //字节序整数表示的IPv4地址
      addr.sin_addr.s_addr = inet_addr(ip);
      if(bind(ser_sock, (struct sockaddr*)&addr, sizeof(addr)) < 0)
      {
           //命名失败
           perror("bind");
           return 2;
      }

      //监听socket
      int listen_sock = listen(ser_sock, 5);
      if(listen_sock < 0)
      {
          //监听失败
          perror("listen");
          return 3;
      }
  
      //接受连接
      //系统调用从listen监听队列中接受一个连接
      //int accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen)  
      //sockfd参数是执行过listen系统调用的监听socket；addr参数用来获取被  
      //接受连接的远端socket地址，该socket地址的长度由addrlen参数指出  
      //accept成功时返回一个新的连接socket，该socket唯一的标识了被接受  
      //的这个连接，服务器可通过读写该socket来与被接受连接对应的客户端通信  
      struct sockaddr_in peer;  
      socklen_t peer_len;  
      char buf[1024];  
      int accept_fd = accept(ser_sock, (struct sockaddr*)&peer, &peer_len);  
  
      if(accept_fd < 0)  
      {  
          perror("accept");  
          return 4;  
      }  
      else  
      {  
          printf("connected with ip: %s  and port: %d\n", inet_ntop(AF_INET,&peer.sin_addr, buf, 1024), ntohs(peer.sin_port));  
          
      }  
  
      while(1)  
      {  
          memset(buf, '\0', sizeof(buf));  
          ssize_t size = read(accept_fd, buf, sizeof(buf) - 1);  
    //    printf("size = %d\n", size);  
    //    printf("buf = %s\n", buf);  
          if(size > 0)  
          {  
              printf("client#: %s\n", buf);  
          }  
          else if(size == 0)  
          {  
              printf("read is done...\n");  
              break;  
          }  
          else   
          {  
              perror("read");  
              break;  
          }  
          printf("server please enter: ");  
          fflush(stdout);  
          size = read(0, buf, sizeof(buf) - 1);  
          if(size > 0)  
          {  
              buf[size - 1] = '\0';  
          }  
          else if(size == 0)  
          {  
              printf("read is done...\n");  
              break;  
          }  
          else  
          {  
              perror("read");  
              break;  
          }     
          write(accept_fd, buf, strlen(buf));  
      }  
        close(ser_sock);  
      return 0;  
}  
