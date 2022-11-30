#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>

#include <netdb.h>
#include <string.h>

#define FTP_PORT 21
#define MAX_LINE_SIZE 256
#define MAX_ARG_SIZE 256


int parse_arguments(char** args, char* username, char* password, char* host, char* url_path){

    char* arg = args[2];
    //check 1st is download
    if(strcmp(args[1],"download") != 0){
        return 1;
    }

    // check protocol
    char* segment;
    segment = strtok(arg, "/");
    if(strcmp(segment,"ftp:") != 0){
        printf("Protocol is invalid!\n");
        return 1;
    }

    char* credentials_host = strtok(NULL, "/");

    // get url_path
    strcpy(url_path,strtok(NULL, "/"));

    // get/check credentials
    char* credentials = strtok(credentials_host,"@");
    strcpy(host, strtok(NULL,"@"));
    strcpy(username, strtok(credentials,":"));
    char* temp_password = strtok(NULL,":");
    if(temp_password == NULL){
        strcpy(password, "");
    }else{
        strcpy(password, temp_password);
    }

    return 0;
}


int getIP(char* host, char* host_ip) {
    struct hostent *h;
    if ((h = gethostbyname(host)) == NULL) {
        herror("gethostbyname()");
        exit(-1);
    }

    printf("Host name  : %s\n", h->h_name);
    printf("IP Address : %s\n", inet_ntoa(*((struct in_addr *) h->h_addr)));
    strcpy(host_ip,inet_ntoa(*((struct in_addr *) h->h_addr)));
    return 0;
}

int open_connect_TCP_socket(int* sockfd, char* server_ip){
    struct sockaddr_in server_addr;

    /*server address handling*/
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip); /*32 bit Internet address network byte ordered*/
    server_addr.sin_port = htons(FTP_PORT);            /*server TCP port must be network byte ordered */

    /*open a TCP socket*/
    if ((*(sockfd) = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket()");
        exit(-1);
    }
    /*connect to the server*/
    if (connect(*(sockfd), (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        perror("connect()");
        exit(-1);
    }
    return 0;
}


int main(int argc, char **argv)
{
    char username[MAX_ARG_SIZE];
    char password[MAX_ARG_SIZE];
    char host[MAX_ARG_SIZE];
    char url_path[MAX_ARG_SIZE];


    int bad_arg = parse_arguments(argv, username, password, host, url_path);

    if (argc != 3 || bad_arg) {
        fprintf(stderr, "Usage: download ftp://[<user>:<password>@]<host>/<url-path>\n");
        exit(-1);
    }
    //printf("%s\n%s\n%s\n%s\n", username, password, host, url_path);
    
    char ip[MAX_ARG_SIZE];
    getIP(host, ip);
    printf("%s",ip);

    int sockfd;
    open_connect_TCP_socket(&sockfd,ip);

/*     FILE *sockf = fdopen(sockfd, "r");
    char ch[200];
    do
    {
        memset(ch, 0, 200);
        fgets(ch, 200, sockf);
        printf("%s", ch);
    } while (!('1' <= ch[0] && ch[0] <= '5') || ch[3] != ' '); */
    return 0;
}
