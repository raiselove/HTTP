#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

void usage(const char *proc)
{
    printf("Usage:%s [ip][port]\n", proc);
}


int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        usage(argv[0]);
        exit(1);
    }
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if(sock < 0)
    {
        perror("sock");
        exit(1);
    }
    int port = atoi(argv[2]);
    struct sockaddr_in remote;
    remote.sin_family = AF_INET;
    remote.sin_port = htons(port);
    remote.sin_addr.s_addr = inet_addr(argv[1]);
    if(connect(sock, (struct sockaddr*)&remote, sizeof(remote)) < 0)
    {
        perror("connect");
        exit(2);
    }
    close(sock);
    return 0;
}
