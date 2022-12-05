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


typedef struct
{
    char command[MAX_ARG_SIZE];
    char protocol[MAX_ARG_SIZE];
    char username[MAX_ARG_SIZE];
    char password[MAX_ARG_SIZE];
    char host[MAX_ARG_SIZE];
    char url_path[MAX_ARG_SIZE];
    char filename[MAX_ARG_SIZE];
} parameters;




int parse_arguments(char **args, parameters* params)
{

    char *arg = args[2];

    // check/get command is download
    if (strcmp(args[1], "download") != 0)
    {
        printf("%s command is invalid!\n", args[1]);
        return 1;
    }
    strcpy(params->command, args[1]);

    // check/get protocol
    char *segment;
    segment = strtok(arg, "/");
    if (strcmp(segment, "ftp:") != 0)
    {
        printf("Protocol is invalid!\n");
        return 1;
    }
    strcpy(params->protocol, segment);

    char *credentials_host = strtok(NULL, "/");

    // get url_path and filename:
    char *dir;
    memset(params->url_path, 0, strlen(params->url_path));
    while ((dir = strtok(NULL, "/")) != NULL)
    {
        strcat(params->url_path, "/");
        strcat(params->url_path, dir);
        strcpy(params->filename, dir);
    }

    char *credentials = strtok(credentials_host, "@");
    // get host
    strcpy(params->host, strtok(NULL, "@"));

    // se input for ftp://"a b":asd]@ftp.up.pt/asdasdasd/asdasd   -> username = "a b"(sem aspas) password = "asd]"
    // se input for ftp://:asd]@ftp.up.pt/asdasdasd/asdasd   -> username = "asd]" password = ""
    // get credentials
    strcpy(params->username, strtok(credentials, ":"));
    char *temp_password = strtok(NULL, ":");
    if (temp_password == NULL)
    {
        strcpy(params->password, "");
    }
    else
    {
        strcpy(params->password, temp_password);
    }

    return 0;
}

int getIP(char *host, char *host_ip)
{
    struct hostent *h;
    if ((h = gethostbyname(host)) == NULL)
    {
        herror("gethostbyname()");
        exit(-1);
    }

    printf("Host name  : %s\n", h->h_name);
    printf("IP Address : %s\n", inet_ntoa(*((struct in_addr *)h->h_addr)));
    strcpy(host_ip, inet_ntoa(*((struct in_addr *)h->h_addr)));
    return 0;
}

int open_connect_TCP_socket(int *sockfd, char *server_ip, int port)
{
    struct sockaddr_in server_addr;

    /*server address handling*/
    bzero((char *)&server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(server_ip); /*32 bit Internet address network byte ordered*/
    server_addr.sin_port = htons(port);                 /*server TCP port must be network byte ordered */

    /*open a TCP socket*/
    if ((*(sockfd) = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket()");
        exit(-1);
    }
    /*connect to the server*/
    if (connect(*(sockfd), (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("connect()");
        exit(-1);
    }
    return 0;
}

int parse_pasv_response(char *response, int *port)
{
    char *token;
    char res[MAX_LINE_SIZE];
    int bytes[6];

    if (response[0] != '2' || response[0] > '5' || response[0] < '1') { return 1;}

    // get last segment of response (with the bytes)
    token = strtok(response, " ");
    while ((token = strtok(NULL, " ")) != NULL) { strcpy(res, token);}

    // construir array com os bytes ex:[193,137,29,15,196,112]
    token = strtok(res, ",");
    for (int i = 0; i < 6; i++){
        if(i == 0) { bytes[0] = atoi(token + 1);}
        else { bytes[i] = atoi(token);}

        token = strtok(NULL, ",");
    }

    // calcular port
    *(port) = bytes[5] + bytes[4] * 256;

    return 0;
}

void print_socket_response(int sockfd)
{
    char line[MAX_LINE_SIZE];
    FILE *sockf = fdopen(sockfd, "r");

    printf("\nSocket response:\n");
    do{
        // memset(line, 0, 200); secalhar nao é necessário
        fgets(line, 200, sockf);
        printf("%s", line);
    } while (!('1' <= line[0] && line[0] <= '5') || line[3] != ' ');
    printf("\n");
    //fechar sockf como? -> acho que não é preciso
}

//gets and prints one line from socket and returns error code
int get_cmd_response(int sockfd, char* response){

    FILE *sockf = fdopen(sockfd, "r");
    sockf = fdopen(sockfd, "r");
    memset(response, 0, MAX_LINE_SIZE);
    fgets(response, MAX_LINE_SIZE, sockf);
    printf("Response: %s\n", response);

    char error_code[3];
    error_code[0] = response[0];
    error_code[1] = response[1];
    error_code[2] = response[2];

    return atoi(error_code);
}

int send_cmd_to_socket(int sockfd, char *cmd, char *arg) //só permite 1 argumento ou 0 (arg="")
{
    char full_cmd[MAX_LINE_SIZE];
    char response[MAX_LINE_SIZE];
    unsigned bytes;

    strcpy(full_cmd, cmd);
    if (strcmp(arg, "") != 0){
        strcat(full_cmd, " ");
        strcat(full_cmd, arg);
    }

    printf("Sending command \"%s\" ...\n", full_cmd);
    strcat(full_cmd, "\n");
    if(bytes = write(sockfd, full_cmd, strlen(full_cmd)) == -1){
        perror("error on write command to sokcet!");
        exit(-1);
    }



    //FALTA INTREPRETAR A RESPOSTA !!!
    return 0;
}




int download_file(int sockfd, char* filename){
    
    printf("Downloading file...\n");
    FILE* f;
    FILE* sockf;

    if((f = fopen(filename, "w")) == NULL) {
        printf("error opening new file\n");
        return 1;
    }
    if((sockf = fdopen(sockfd, "r")) == NULL) {
        printf("error opening socket file;\n");
        return 1;
    }

    char c;
    c = fgetc(sockf);
    while (c != EOF){
        fputc(c, f);
        c = fgetc(sockf);
    }

    fclose(f);
    //fechar sockf como? -> acho que não é preciso
}

int main(int argc, char **argv)
{
    parameters params;
    char ip[MAX_ARG_SIZE];
    char response[MAX_LINE_SIZE];
    int sockfd, datasocketfd, port;
    unsigned bytes; // NOT USEFULL ?!!!!!!!!!!!!!!!!!!!!!!?

    if (argc != 3 || parse_arguments(argv, &params))
    {
        fprintf(stderr, "Usage: download ftp://[<user>:<password>@]<host>/<url-path>\n");
        exit(-1);
    }
    printf("username   : %s\npassword   : %s\nhost       : %s\nurl_path   : %s\nfilename   : %s\n\n", params.username, params.password, params.host, params.url_path, params.filename);

    //get IP of host
    getIP(params.host, ip);

    open_connect_TCP_socket(&sockfd, ip, FTP_PORT);

    // print socket response
    print_socket_response(sockfd);

    // send command (username)
    send_cmd_to_socket(sockfd, "user", params.username);
    // print command response
    get_cmd_response(sockfd, response);

    // send command (password)
    send_cmd_to_socket(sockfd, "pass ", params.password);
    // print command response
    get_cmd_response(sockfd, response);

    // send pasv command
    send_cmd_to_socket(sockfd, "pasv", params.password);
    // print/get command (pasv) response
    get_cmd_response(sockfd, response);
    //printf("error_code: %u\n", get_cmd_response(sockfd, response));
    // get port from pasv command response
    parse_pasv_response(response, &port);
    printf("Data socket port: %u\n", port);

    // open data socket
    open_connect_TCP_socket(&datasocketfd, ip, port);

    // send command (retr)
    send_cmd_to_socket(sockfd, "retr", params.url_path);
    // print command response
    get_cmd_response(sockfd, response);

    // close control socket
    if (close(sockfd) < 0){
        perror("close()");
        exit(-1);
    }

    // read file transfered
    download_file(datasocketfd, params.filename);

    // close data socket
    if (close(datasocketfd) < 0){
        perror("close()");
        exit(-1);
    }

    printf("File Downloaded with success!\n");
    return 0;
}
