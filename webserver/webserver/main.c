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
    // insert code here...
    int sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    drop(sock,"socket error");
    return 0;
}
