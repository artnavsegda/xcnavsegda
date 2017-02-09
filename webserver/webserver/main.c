#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/param.h>

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

void drop3(long dropstatus, char *dropdesc)
{
    if (dropstatus == -1)
    {
        perror(dropdesc);
        exit(1);
    }
}

long long filesize(int fd)
{
    struct stat filestat;
    drop(fstat(fd,&filestat),"file status error");
    return filestat.st_size;
}

void pwd(void)
{
    char path[MAXPATHLEN];
    getwd(path);
    printf("wd: %s\n",path);
}

int main(int argc, const char * argv[]) {
    pwd();
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
        drop(msgsock,"accept error");
        int nchar = 0;
        while (nchar == 0)
            ioctl(msgsock,FIONREAD,&nchar);
        char *buf = malloc(nchar+1);
        long numread = recv(msgsock,buf,nchar,0);
        drop3(numread,"recv error");
        buf[numread] = '\0';
        strtok(buf," ");
        char *page = strtok(NULL," ");
        if (strcmp(page,"/") == 0)
            page = "/index.html";
        int webpage = open(&page[1],O_RDONLY);
        if (webpage == -1)
        {
            drop3(send(msgsock,"HTTP/1.1 404 Not Found",22,0),"send response error");
            drop3(send(msgsock,"\nContent-type: text/html\n\n",26,0),"send mime type error");
            drop3(send(msgsock,"<!doctype html><html><head><title>404 Not Found</title></head><body><p>page not found</p></body></html>",103,0),"send webpage error");
        }
        else
        {
            drop3(send(msgsock,"HTTP/1.1 200 OK",15,0),"send response error");
            drop3(send(msgsock,"\nContent-type: text/html\n\n",26,0),"send mime type error");
            char *data = malloc(filesize(webpage)+1);
            numread = read(webpage,data,filesize(webpage));
            drop3(numread,"read error");
            data[numread] = '\0';
            close(webpage);
            drop3(send(msgsock,data,strlen(data),0),"send webpage error");
            free(data);
        }
        free(buf);
        shutdown(msgsock,2);
        close(msgsock);
    }
    return 0;
}
