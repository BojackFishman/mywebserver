#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>

int main(){

    // 创建套接字
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd == -1){
        perror("socket");
        exit(-1);
    }

    // 连接服务器
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    inet_pton(AF_INET, "192.168.44.128", &serverAddr.sin_addr.s_addr);
    serverAddr.sin_port = htons(4000);
    int ret = connect(sockfd, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    if(ret == -1){
        perror("connect");
        exit(-1);
    }


    char recvBuf[1024] = {0};
    char* data = "this is client\n";
    int i = 0;
    while(1){
        sprintf(recvBuf, "data: %d\n", i++);
        write(sockfd, recvBuf, strlen(recvBuf));

        int len = read(sockfd, recvBuf, sizeof(recvBuf));
        if(len == -1){
            perror("read");
            exit(-1);
        }
        else if(len > 0){
            printf("recv server data: %s \n", recvBuf);
        }
        else if(len == 0){
            printf("server closed...");
            break;
        }
        sleep(1);
    }
    

    close(sockfd);

    return 0;
}