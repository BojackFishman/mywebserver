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
#include<poll.h>
#include<sys/epoll.h>


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

    // 初始化检测的文件描述符数组
    // struct pollfd fds[1024];
    // for(int i = 0; i < 1024; ++i){
    //     fds[i].fd = -1;
    //     fds[i].events = POLLIN;
    // }
    // fds[0].fd = httpd;
    // int nfds = 0;

    // 创建一个epoll实例
    int epfd = epoll_creat(100);
    // 将监听socket fd加入epoll实例中
    struct epoll_event epev;
    epev.events = POLLIN;
    epev.data.fd = httpd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, httpd, &epev);

    struct epoll_event epevs[1024];

    while(1){

        // 调用poll()检测文件描述符数据
        // ret = poll(fds, nfds + 1, -1);
        ret = epoll_wait(epfd, &epevs, 1024, -1);
        if(ret == -1){
            perror("epll_wait");
            exit(-1);
        }
        printf("ret = %d\n", ret);
        // 检测到了有文件描述符对应缓冲区数据发生了改变
        for(int i = 0; i < ret; ++i){
            int curfd = epevs[i].data.fd;

            if(curfd == httpd){
                // httpd有数据，有新的客户端连接进来
                struct sockaddr_in clientaddr;
                socklen_t client_addr_len = sizeof(clientaddr);
                int clientfd;
                clientfd = accept(httpd, (struct sockaddr *) & clientaddr, &client_addr_len);

                epev.events = EPOLLIN;
                epev.data.fd = clientfd;
                epoll_ctl(epfd, EPOLL_CTL_ADD, clientfd, &epev);
                // 将新的文件描述符加入集合中
                // 相比select的位操作，poll还需O(n)检测找一个空的fds槽位
                // 注意这里的i变成了fds数组的索引，而不再指代文件描述符
                // for(int i = 1; i < 1024; ++i){
                //     if(fds[i].fd == -1){
                //         // 先检索到有fd的空槽然后再接收连接。否则可能出现接收了连接无法放入fds中的情况
                //         // 如果没有空槽了那么就等下一次poll再进行连接。但是没有取走httpd缓冲区的数据下一次还会再提醒吗
                //         fds[i].fd = clientfd;
                //         fds[i].events = POLLIN;
                //         nfds = nfds > i ? nfds : i;
                //         break;
                //     }
                // }
            }
            else {
                char recvBuf[1024] = {0};
                int len = read(curfd, &recvBuf, sizeof(recvBuf));
                if(len == -1){
                    perror("read");
                    exit(-1);
                }
                else if(len > 0){
                    printf("recv client data: %s \n", recvBuf);
                    write(curfd, recvBuf, strlen(recvBuf)+1);
                }
                else if(len == 0){
                    printf("client closed...\n");
                    epoll_ctl(epfd, EPOLL_CTL_DEL, curfd, NULL);
                    close(curfd);
                    // FD_CLR(i, &rdset);
                    // fd[i].fd = -1;
                }
            }
        }
    }
    // 关闭文件描述符
    close(httpd);

    close(epfd);

    return 0;
}