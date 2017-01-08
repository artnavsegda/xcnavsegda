#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

unsigned short hrmassive[50];

unsigned char crmassive[20];

unsigned short generatecoils(int firstcoil, int numberofcoils)
{
    return crmassive[0]>>firstcoil;
}

unsigned char buf[100];

unsigned char data[12] = { 0x00, 0x01, 0x00, 0x00, 0x00, 0x05, 0x32, 0x03, 0x02, 0x00, 0x00 };

struct askreadregstruct {
    unsigned short firstreg;
    unsigned short regnumber;
};

struct reqreadstruct {
    unsigned char bytestofollow;
    unsigned char bytes[254];
};

struct writeregstruct {
    unsigned short regaddress;
    unsigned short regvalue;
};

struct writemulticoilstruct {
    unsigned short firstreg;
    unsigned short regnumber;
    unsigned char bytestofollow;
    unsigned char coils[256];
};

struct writemultiregstruct {
    unsigned short firstreg;
    unsigned short regnumber;
    unsigned char bytestofollow;
    unsigned short registers[127];
};

union pdudataunion {
    struct askreadregstruct askreadregs;
    struct reqreadstruct reqread;
    struct writeregstruct writereg;
    struct writemulticoilstruct writemulticoil;
    struct writemultiregstruct writemultireg;
    unsigned short words[127];
    unsigned char bytes[254];
};

struct pduframestruct {
    unsigned char unitid;
    unsigned char fncode;
    union pdudataunion data;
};

struct mbframestruct {
    unsigned short tsid;
    unsigned short protoid;
    unsigned short length;
    struct pduframestruct pdu;
};

struct mbframestruct askmbframe, reqmbframe;

unsigned short table[100] = {0xABCD, 0xDEAD};
unsigned short amount = 100;

int main()
{
    int sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if (sock == -1)
    {
        perror("socket error");
        return 1;
    }
    else
    {
        printf("socket ok\n");
    }
    int optval = 1;
    setsockopt(sock,SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    
    struct sockaddr_in server = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(1100)
    };
    
    if (bind(sock,(struct sockaddr *)&server,sizeof(server)) == -1)
    {
        perror("bind error");
        close(sock);
        return 1;
    }
    else
    {
        printf("bind ok\n");
    }
    
    if (listen(sock,10) == -1)
    {
        perror("listen error");
        close(sock);
        return 1;
    }
    else
    {
        printf("listen ok\n");
    }
    
    while(1)
    {
        int msgsock = accept(sock,NULL,NULL);
        if (msgsock == -1)
        {
            perror("accept error");
            close(sock);
            return 1;
        }
        else
        {
            printf("accept ok\n");
        }
        
        ssize_t numread = recv(msgsock,&askmbframe,6,0);
        if (numread == -1)
        {
            perror("recv error");
            close(msgsock);
            close(sock);
            return 1;
        }
        else
        {
            printf("recv %zd bytes\n",numread);
            printf("TS id: %d\n", ntohs(askmbframe.tsid));
            printf("Protocol id: %d\n", ntohs(askmbframe.protoid));
            printf("Length: %d\n", ntohs(askmbframe.length));
            /*for (int i=0; i<numread;i++)
             {
             printf("0x%02X ",buf[i]);
             }
             printf("\n");*/
        }
        
        numread = recv(msgsock,&askmbframe.pdu,ntohs(askmbframe.length),0);
        if (numread == -1)
        {
            perror("recv error");
            close(msgsock);
            close(sock);
            return 1;
        }
        else
        {
            printf("recv %zd bytes\n",numread);
            printf("Unit id: %d\n", askmbframe.pdu.unitid);
            printf("Function code: %d\n", askmbframe.pdu.fncode);
            for (int i=0; i<(numread-2)/2;i++)
                printf("%u ",ntohs(askmbframe.pdu.data.words[i]));
            printf("\n");
        }
        
        int firstrequest = 0;
        int requestnumber = 0;
        switch (askmbframe.pdu.fncode) {
            case 1:
            case 2:
                askmbframe.pdu.data.reqread.bytestofollow = ntohs(askmbframe.pdu.data.askreadregs.regnumber) / 8;
                if ((ntohs(askmbframe.pdu.data.askreadregs.regnumber) % 8)>0)
                    askmbframe.pdu.data.reqread.bytestofollow++;
                askmbframe.length = htons(askmbframe.pdu.data.reqread.bytestofollow + 3);
                // fill all requested coil bytes with zeroes
                for (int i = 0; i < askmbframe.pdu.data.reqread.bytestofollow; i++)
                    askmbframe.pdu.data.reqread.bytes[i] = 0x00;
                break;
            case 3:
            case 4:
                firstrequest = ntohs(askmbframe.pdu.data.askreadregs.firstreg);
                printf("Requesing register starting from: %d\n", firstrequest);
                requestnumber = ntohs(askmbframe.pdu.data.askreadregs.regnumber);
                printf("Number of registers requested: %d\n", requestnumber);
                askmbframe.pdu.data.reqread.bytestofollow = requestnumber * 2;
                askmbframe.length = htons(askmbframe.pdu.data.reqread.bytestofollow + 3);
                // fill every requested register with table values
                for (int i = 0; i < requestnumber;i++)
                    if (firstrequest+i < amount)
                        ((unsigned short *)&askmbframe.pdu.data.reqread.bytes)[i] = htons(table[firstrequest+i]);
                    else
                        ((unsigned short *)&askmbframe.pdu.data.reqread.bytes)[i] = htons(0x0000);
                /*for (int i = 0; i < requestnumber;i++)
                 ((unsigned short *)&askmbframe.pdu.data.reqread.bytes)[i] = htons(table[i]);*/
                break;
            case 5:
                //same as request
                break;
            case 6:
                if (ntohs(askmbframe.pdu.data.writereg.regaddress) < amount)
                    table[ntohs(askmbframe.pdu.data.writereg.regaddress)] = ntohs(askmbframe.pdu.data.writereg.regvalue);
                break;
            case 15:
                askmbframe.length = htons(6);
                break;
            case 16:
                for (int i = 0; i<ntohs(askmbframe.pdu.data.writemultireg.regnumber);i++)
                    if(ntohs(askmbframe.pdu.data.writemultireg.firstreg)+i < amount)
                        table[ntohs(askmbframe.pdu.data.writemultireg.firstreg)+i] = ntohs(askmbframe.pdu.data.writemultireg.registers[i]);
                askmbframe.length = htons(6);
                break;
                break;
        }
        
        int replylength = ntohs(askmbframe.length) + 6;
        printf("replylength %d\n",replylength);
        
        ssize_t numwrite = send(msgsock,&askmbframe,replylength,0);
        if (numwrite == -1)
        {
            perror("send error");
            close(msgsock);
            close(sock);
            return 1;
        }
        else
        {
            printf("send %zd bytes ok\n",numwrite);
        }
        
        shutdown(msgsock, 2);
        /*if (shutdown(msgsock, 2) == -1)
         {
         perror("shutdown error");
         close(msgsock);
         close(sock);
         return 1;
         }
         else
         {
         printf("shutdown ok\n");
         }*/
        close(msgsock);
    }
    
    return 0;
}
