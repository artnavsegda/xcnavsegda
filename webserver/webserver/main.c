#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>

void drop(int dropstatus, char *dropdesc)
{
    if (dropstatus == -1)
    {
        perror(dropdesc);
        exit(1);
    }
}

void drop2(char *dropstatus, char *dropdesc)
{
    if (dropstatus == NULL)
    {
        perror(dropdesc);
        exit(1);
    }
}

int main(int argc, const char * argv[]) {
    int sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    drop(sock,"socket error");
    int reuseaddr = 1;
    setsockopt(sock,SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr));
    struct sockaddr_in server = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(1100)
    };
    drop(bind(sock,(struct sockaddr *)&server,sizeof(server)),"bind error");
    drop(listen(sock,10),"listen error");
    while (1)
    {
        int msgsock = accept(sock,NULL,NULL);
        
    }
    return 0;
}
