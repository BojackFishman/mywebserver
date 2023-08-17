#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>


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

    // 监听listen
    ret = listen(httpd, 5);
    if(ret == -1){
        perror("listen");
        exit(-1);
    }

    // 接收客户端连接
    struct sockaddr_in clientaddr;
    socklen_t client_addr_len = sizeof(clientaddr);
    int clientfd;
    clientfd = accept(httpd, (struct sockaddr *) & clientaddr, &client_addr_len);
    if(clientfd == -1){
        perror("accept");
        exit(-1);
    }

    // 输出客户端信息
    char clienIP[16];
    inet_ntop(AF_INET, &clientaddr.sin_addr.s_addr, clienIP, sizeof(clienIP));
    unsigned short clientPort = ntohs(clientaddr.sin_port);
    printf("client ip : %s, port : %d\n", clienIP, clientPort);

    char recvBuf[1024] = {0};
    char* data = "this is server/n";

    // 获取客户端数据
    while(1){
        int len = read(clientfd, recvBuf, sizeof(recvBuf));
        if(len == -1){
            perror("read");
            exit(-1);
        }
        else if(len > 0){
            printf("recv client data: %s \n", recvBuf);
        }
        else if(len == 0){
            printf("client closed...");
            break;
        }

        write(clientfd, data, strlen(data));
    }
    

    // 关闭文件描述符
    close(clientfd);
    close(httpd);

    return 0;
}