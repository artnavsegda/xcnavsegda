#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

char buf[100];
char ask[12] = { 0x00, 0x01, 0x00, 0x00, 0x00, 0x06, 0x32, 0x03, 0x00, 0x00, 0x00, 0x01 };

struct tcpframestruct {
    unsigned short tsid;
    unsigned short protoid;
    unsigned short length;
};

struct askreadregstruct {
    unsigned short firstreg;
    unsigned short regnumber;
};

struct writeregstruct {
    unsigned short regaddress;
    unsigned short regvalue;
};

struct reqreadstruct {
    unsigned char bytestofollow;
    unsigned char bytes[254];
};

struct writemulticoilstruct {
    unsigned short firstreg;
    unsigned short regnumber;
    unsigned char bytestofollow;
    unsigned char coils[256];
};

union pdudataunion {
    struct askreadregstruct askreadregs;
    struct reqreadstruct reqread;
    struct writeregstruct writereg;
    struct writemulticoilstruct writemulticoil;
    unsigned short words[127];
};

struct pduframestruct {
    unsigned char unitid;
    unsigned char fncode;
    union pdudataunion data;
};

struct tcpframestruct tcpframe = {
    .tsid = 1,
    .protoid = 0,
    .length = 6,
};

struct pduframestruct pduframe = {
    .unitid = 50,
    .fncode = 3,
};

//struct tcpframestruct askframe;
//struct pduframestruct askpduframe;

struct mbframestruct {
    unsigned short tsid;
    unsigned short protoid;
    unsigned short length;
    struct pduframestruct pdu;
    //	unsigned char unitid;
    //	unsigned char fncode;
    //	unsigned short data[2];
};

struct mbframestruct mbframe = {
    .tsid = 0x0100,
    .protoid = 0x0000,
    .length = 0x0600,
    .pdu = {
        .unitid = 50
    }
    //	.unitid = 50,
    //	.fncode = 3,
    //	.data = { 0x0000, 0x0100 }
};

struct mbframestruct askframe;

void printbinary(unsigned char value)
{
    for (int i = 0; i < 8; i++)
        printf("%u", (value >> (7-i)) & 0x01);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        printf("name function code and its values\n");
        printf("1 first amount: read coil\n");
        printf("2 first amount: read discrete\n");
        printf("3 first amount: read input\n");
        printf("4 first amount: read holding\n");
        printf("5 number value: write coil\n");
        printf("6 number value: write holding\n");
        printf("15 first amount number value: write miltiple coil\n");
        printf("16 first amount number value: write miltiple holding\n");
        return 1;
    }
    
    sscanf(argv[1],"%hhu",&mbframe.pdu.fncode); // <-----
    
    switch (mbframe.pdu.fncode)
    {
        case 1:
        case 2:
            if (argc < 4)
            {
                printf("name first coil/discrete and amount to return\n");
                return 1;
            }
            sscanf(argv[2],"%hu",&mbframe.pdu.data.askreadregs.firstreg);
            mbframe.pdu.data.askreadregs.firstreg = htons(mbframe.pdu.data.askreadregs.firstreg);
            sscanf(argv[3],"%hu",&mbframe.pdu.data.askreadregs.regnumber);
            mbframe.pdu.data.askreadregs.regnumber = htons(mbframe.pdu.data.askreadregs.regnumber);
            break;
        case 3:
        case 4:
            if (argc < 4)
            {
                printf("name first input/holding and amount to return\n");
                return 1;
            }
            sscanf(argv[2],"%hu",&mbframe.pdu.data.askreadregs.firstreg);
            mbframe.pdu.data.askreadregs.firstreg = htons(mbframe.pdu.data.askreadregs.firstreg);
            sscanf(argv[3],"%hu",&mbframe.pdu.data.askreadregs.regnumber);
            mbframe.pdu.data.askreadregs.regnumber = htons(mbframe.pdu.data.askreadregs.regnumber);
            break;
        case 5:
        case 6:
            if (argc < 4)
            {
                printf("name register number and value to set\n");
                return 1;
            }
            sscanf(argv[2],"%hu",&mbframe.pdu.data.writereg.regaddress);
            mbframe.pdu.data.writereg.regaddress = htons(mbframe.pdu.data.writereg.regaddress);
            sscanf(argv[3],"%hu",&mbframe.pdu.data.writereg.regvalue);
            mbframe.pdu.data.writereg.regvalue = htons(mbframe.pdu.data.writereg.regvalue);
            break;
        case 15:
        case 16:
            if (argc < 5)
            {
                printf("name first register, registers amount to set, number of bytes to follow and bytes with values\n");
                return 1;
            }
            sscanf(argv[2],"%hu",&mbframe.pdu.data.writemulticoil.firstreg);
            mbframe.pdu.data.writemulticoil.firstreg = htons(mbframe.pdu.data.writemulticoil.firstreg);
            sscanf(argv[3],"%hu",&mbframe.pdu.data.writemulticoil.regnumber);
            mbframe.pdu.data.writemulticoil.regnumber = htons(mbframe.pdu.data.writemulticoil.regnumber);
            sscanf(argv[4],"%hhu",&mbframe.pdu.data.writemulticoil.bytestofollow);
            for (int i=0;i<mbframe.pdu.data.writemulticoil.bytestofollow;i++)
                sscanf(argv[4+i],"%hhu",&mbframe.pdu.data.writemulticoil.coils[i]);
            break;
        default:
            printf("unknown function number");
            return 1;
            break;
    }
    
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1)
    {
        perror("socket error");
        return 1;
    }
    else
    {
        printf("socket ok\n");
    }
    struct sockaddr_in client = {
        .sin_addr.s_addr = inet_addr("127.0.0.1"),
        .sin_family = AF_INET,
        .sin_port = htons(1100)
    };
    if (connect(sock,(struct sockaddr *)&client, sizeof(client)) == -1)
    {
        perror("connect error");
        close(sock);
        return 1;
    }
    else
    {
        printf("connect ok\n");
    }
    
    ssize_t numwrite = send(sock,&mbframe,12,0);
    if (numwrite == -1)
    {
        perror("send error");
    }
    else
    {
        printf("send %zd bytes ok\n", numwrite);
    }
    
    //int numread = recv(sock,buf,100,0);
    ssize_t numread = recv(sock,&askframe,6,0);
    if (numread == -1)
    {
        perror("recv error");
        close(sock);
        return 1;
    }
    else
    {
        printf("recv %zd bytes\n",numread);
        printf("TS id: %d\n", ntohs(askframe.tsid));
        printf("Protocol id: %d\n", ntohs(askframe.protoid));
        printf("Length: %d\n", ntohs(askframe.length));
        /*for (int i=0; i<numread;i++)
         {
         printf("0x%02hhX ",buf[i]);
         }
         printf("\n");*/
    }
    
    numread = recv(sock,&askframe.pdu,ntohs(askframe.length),0);
    if (numread == -1)
    {
        perror("recv error");
        close(sock);
        return 1;
    }
    else
    {
        printf("recv %zd bytes\n",numread);
        printf("Unit id: %d\n", askframe.pdu.unitid);
        printf("Function code: %d\n", askframe.pdu.fncode);
        for (int i=0; i<numread;i++)
            printf("0x%02hhX ",((char *)&askframe.pdu)[i]);
        printf("\n");
        switch(askframe.pdu.fncode)
        {
            case 1:
            case 2:
                printf("number of bytes containing bit values: %d\n",askframe.pdu.data.reqread.bytestofollow);
                for (int i=0;i<askframe.pdu.data.reqread.bytestofollow;i++)
                {
                    printf("0x%02hhX(",askframe.pdu.data.reqread.bytes[i]);
                    printbinary(askframe.pdu.data.reqread.bytes[i]);
                    printf(") ");
                }
                printf("\n");
                break;
            case 3:
            case 4:
                printf("number of registers: %d\n",askframe.pdu.data.reqread.bytestofollow/2);
                for (int i=0;i<askframe.pdu.data.reqread.bytestofollow/2;i++)
                {
                    printf("0x%04hX ",((short *)&askframe.pdu.data.reqread.bytes)[i]);
                }
                printf("\n");
                break;
            case 5:
            case 6:
                printf("register number %u\n", ntohs(askframe.pdu.data.writereg.regaddress));
                printf("register value %u\n", ntohs(askframe.pdu.data.writereg.regvalue));
                break;
            case 15:
            case 16:
                printf("first register %u\n", ntohs(askframe.pdu.data.writemulticoil.firstreg));
                printf("registers amount %u\n", ntohs(askframe.pdu.data.writemulticoil.regnumber));
                break;
            default:
                printf("unknown function number");
                break;
        }
    }
    
    if (shutdown(sock, 2) == -1)
    {
        perror("shutdown error");
        close(sock);
        return 1;
    }
    else
    {
        printf("shutdown ok\n");
    }
    close(sock);
    
    return 0;
}
