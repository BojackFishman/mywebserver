#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<errno.h>
#include<pthread.h>
#include<sys/select.h>


int main(){

    int httpd = 0;

    // 创建socket，用于监听
    httpd = socket(AF_INET, SOCK_STREAM, 0);
    if(httpd == -1){
        perror("socket");
        exit(-1);
    }

    // bind
    struct sockaddr_in saddr;
    saddr.sin_family = AF_INET;
    int port = 4000;
    saddr.sin_port = htons(port);
    // inet_pton(AF_INET, "192.168.44.128", saddr.sin_addr.s_addr); 
    saddr.sin_addr.s_addr = INADDR_ANY;
    int ret = bind(httpd, (struct sockaddr *) &saddr, sizeof(saddr));
    if(ret == -1){
        perror("bind");
        exit(-1);
    }

    // listen
    listen(httpd, 5);

    // 创建一个fd_set的集合，存放要检测的文件描述符
    fd_set rdset, tmp;
    FD_ZERO(&rdset);
    FD_SET(httpd, &rdset);
    int maxfd = httpd;


    while(1){

        tmp = rdset;
        // 调用select()进行监听
        ret = select(maxfd+1, &tmp, NULL, NULL, NULL);
        if(ret == -1){
            perror("select");
            exit(-1);
        }else if(ret == 0){
            continue;
        }else if(ret > 0){
            // 检测到了有文件描述符对应缓冲区数据发生了改变
            if(FD_ISSET(httpd, &tmp)){
                // httpd有数据，有新的客户端连接进来
                struct sockaddr_in clientaddr;
                socklen_t client_addr_len = sizeof(clientaddr);
                int clientfd;
                clientfd = accept(httpd, (struct sockaddr *) & clientaddr, &client_addr_len);
                FD_SET(clientfd, &rdset);

                maxfd = maxfd > clientfd ? maxfd : clientfd;
            }
            // 轮询
            for(int i = httpd + 1; i <= maxfd; ++i){
                if(FD_ISSET(i, &tmp)){
                    // fd=i的客户端发来了数据
                    char recvBuf[1024] = {0};
                    int len = read(i, &recvBuf, sizeof(recvBuf));
                    if(len == -1){
                        perror("read");
                        exit(-1);
                    }
                    else if(len > 0){
                        printf("recv client data: %s \n", recvBuf);
                        write(i, recvBuf, strlen(recvBuf)+1);
                    }
                    else if(len == 0){
                        printf("client closed...\n");
                        close(i);
                        FD_CLR(i, &rdset);
                    }


                }
            }
        }
    }
    // 关闭文件描述符
    close(httpd);

    return 0;
}