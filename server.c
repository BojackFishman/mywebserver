#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<stdio.h>
#include<unistd.h>
#include<string.h>
#include<stdlib.h>
#include<errno.h>
#include<pthread.h>


struct sockInfo
{
    int sockfd;
    struct sockaddr_in addr;
    pthread_t tid;
};

// 存储传入线程的参数数据。如果定义为局部变量一层循环结束就会销毁。并且要支持可能的并发访问。
// 线程的栈空间资源不共享，管理堆空间或许又比较复杂；教程说的，真的复杂吗？
struct sockInfo sockinfos[128];

// 子线程与客户端通信
void* working(void* arg){
    struct sockInfo* pinfo = (struct sockInfo*)arg;

    // 输出客户端信息
    char clienIP[16];
    inet_ntop(AF_INET, &pinfo->addr.sin_addr.s_addr, clienIP, sizeof(clienIP));
    unsigned short clientPort = ntohs(pinfo->addr.sin_port);
    printf("client ip : %s, port : %d\n", clienIP, clientPort);

    char recvBuf[1024] = {0};
    char* data = "this is server\n";

    // 获取客户端数据
    while(1){
        // printf("%d\n", pinfo->sockfd);
        int len = read(pinfo->sockfd, &recvBuf, sizeof(recvBuf));
        if(len == -1){
            perror("read");
            exit(-1);
        }
        else if(len > 0){
            printf("recv client data: %s \n", recvBuf);
        }
        else if(len == 0){
            printf("client closed...\n");
            break;
        }

        write(pinfo->sockfd, recvBuf, strlen(recvBuf)+1);
    }
    close(pinfo->sockfd);

    // 不同。关闭传入数据槽的占用
    pinfo->sockfd = -1;
}

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

    // 初始化线程传入参数
    int arr_len = sizeof(sockinfos)/sizeof(sockinfos[0]);
    memset(&sockinfos, 0, sizeof(sockinfos));
    for(int i = 0; i < arr_len; ++i){
        sockinfos[i].sockfd = -1;
        sockinfos[i].tid = -1;
    }

    // 接收客户端连接
    // 循环等待客户端连接
    while(1){
        struct sockaddr_in clientaddr;
        socklen_t client_addr_len = sizeof(clientaddr);
        int clientfd;
        clientfd = accept(httpd, (struct sockaddr *) & clientaddr, &client_addr_len);
        if(clientfd == -1){
            if(errno == EINTR){
                continue;
            }
            perror("accept");
            exit(-1);
        }

        // 每一个连接进来，创建一个子线程跟客户端通信
        struct sockInfo *pinfo;
        for(int i = 0; i < arr_len; ++i){
            // 分配一个可用的槽
            if(sockinfos[i].sockfd == -1){
                pinfo = &sockinfos[i];
                break;
            }
            if(i == arr_len-1){
                sleep(1);
                i = -1;
            }
        }
        pinfo->sockfd = clientfd;
        pinfo->addr = clientaddr;
        // memcmp(&pinfo->addr, &clientaddr, client_addr_len);

        pthread_create(&pinfo->tid, NULL, working, pinfo);

        pthread_detach(pinfo->tid);

        // close(clientfd);
    }

    // 关闭文件描述符
    close(httpd);

    return 0;
}