#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>

#define FTP_PORT 21
#define MAX_LINE_SIZE 256




int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: download ftp://[<user>:<password>@]<host>/<url-path>\n");
        exit(-1);
    }






    return 0;
}
