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
#define DEFAULT_USERNAME "anonymous"
#define DEFAULT_PASSWORD ""


typedef struct
{
    char command[MAX_ARG_SIZE];
    char protocol[MAX_ARG_SIZE];
    char username[MAX_ARG_SIZE];
    char password[MAX_ARG_SIZE];
    char host[MAX_ARG_SIZE];
    char url_path[MAX_ARG_SIZE];
    char filename[MAX_ARG_SIZE];
    char ip[MAX_ARG_SIZE];
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

    //check if credentials are present on the string
    //if not default credentials are used
    strcpy(params->username, DEFAULT_USERNAME);
    strcpy(params->password, DEFAULT_PASSWORD);

    if(strchr(credentials_host,'@') == NULL){
        strcpy(params->host, credentials_host);
        return 0;
    }

    char *credentials = strtok(credentials_host, "@");

    // get host
    strcpy(params->host, strtok(NULL, "@"));

    // get credentials
    if(strcmp(credentials, ":") == 0){
        return 0;
    }
    else if(credentials[0] == ':'){
        strcpy(params->password, strtok(credentials, ":"));
        return 0;
    }
    strcpy(params->username, strtok(credentials, ":"));
    char *temp_password = strtok(NULL, ":");
    if (temp_password != NULL){
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

int parse_pasv_response(char *response, int *port, char* ip)
{
    char *token;
    char* usable_ip;
    char res[MAX_LINE_SIZE];
    int bytes[6];
    strcpy(usable_ip, ip);

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

    // Checkar IP Recebido pelo comando pasv com o dos argumentos
    token = strtok(usable_ip, ".");
    for (int i = 0; i < 4; i++){
        if(atoi(token) != bytes[i]){
            printf("IP recebido pelo comando \"pasv\" diferente do ip do host identificado!\n");
            return -1;
        }
        token = strtok(NULL, ".");
    }
    printf("IP recebido pelo comando \"pasv\" est?? correto!\n");

    // calcular port
    *(port) = bytes[5] + bytes[4] * 256;
    printf("Data socket port: %u\n\n", *(port));
    return 0;
}

int parse_retr_response(char *response, int *bytes)
{
    char *token;
    char res[MAX_LINE_SIZE];

    // get last segment of response (with the bytes)
    token = strtok(response, " ");
    while ((token = strtok(NULL, " ")) != NULL) { 
        if(token[0] == '('){
            strcpy(res, token);
        }
    }
    if(res[0] != '(') {return -1;}

    // get bytes sent to data packet
    *(bytes) = atoi(strtok(res, " ") + 1);

    return 0;
}

void print_socket_response(int sockfd)
{
    char line[MAX_LINE_SIZE];
    FILE *sockf;
    if((sockf = fdopen(sockfd, "r")) == NULL) {
        printf("error opening socket file;\n");
        exit(-1);
    }

    printf("\nSocket response:\n");
    do{
        fgets(line, 200, sockf);
        printf("%s", line);
    } while (!('1' <= line[0] && line[0] <= '5') || line[3] != ' ');
    printf("\n");
    
    return;
}

//gets and prints one line from socket and returns error code
int get_cmd_response(int sockfd, char* response){

    FILE *sockf;
    if((sockf = fdopen(sockfd, "r")) == NULL) {
        printf("error opening socket file;\n");
        return 1;
    }
    memset(response, 0, strlen(response));
    fgets(response, MAX_LINE_SIZE, sockf);
    if (response[0] > '5' || response[0] < '1') { return -1;}

    printf("Response: %s\n", response);

    char error_code[3];
    error_code[0] = response[0];
    error_code[1] = response[1];
    error_code[2] = response[2];

    return atoi(error_code);
}

//sends command to socket, gets the socket reponse and returns the reponse's errorcode
int send_cmd_to_socket(int sockfd, char *cmd, char *arg, char* response) //s?? permite 1 argumento ou 0 (arg="")
{
    char full_cmd[MAX_LINE_SIZE];
    unsigned bytes;
    memset(response, 0, strlen(response));

    strcpy(full_cmd, cmd);
    if (strcmp(arg, "") != 0){
        strcat(full_cmd, " ");
        strcat(full_cmd, arg);
    }

    printf("Sending command \"%s\" ...\n", full_cmd);
    strcat(full_cmd, "\n");
    if(bytes = write(sockfd, full_cmd, strlen(full_cmd)) == -1){
        printf("error on write command to socket!");
        exit(-1);
    }

    return get_cmd_response(sockfd, response);
}

int download_file(int sockfd, char* filename){
    
    printf("Downloading file...\n");
    int bytes_received = 0;
    FILE* f;
    FILE* sockf;

    if((f = fopen(filename, "w")) == NULL) {
        printf("error opening new file\n");
        return -1;
    }
    if((sockf = fdopen(sockfd, "r")) == NULL) {
        printf("error opening socket file;\n");
        return -1;
    }

    char c;
    while (read(sockfd, &c, 1) > 0){
        fputc(c, f);
        bytes_received++;
    }

    fclose(f);
    return bytes_received;
}


int main(int argc, char **argv)
{
    parameters params;
    char response[MAX_LINE_SIZE];
    int control_socketfd, data_socketfd, port, error_code;
    int bytes_sent, bytes_received;

    if (argc != 3 || parse_arguments(argv, &params)){
        fprintf(stderr, "Usage: download ftp://[<user>:<password>@]<host>/<url-path>\n");
        exit(-1);
    }
    printf("username   : %s\n", params.username);
    printf("password   : %s\n", params.password);
    printf("host       : %s\n", params.host);
    printf("url_path   : %s\n", params.url_path);
    printf("filename   : %s\n", params.filename);

    //get IP of host
    getIP(params.host, params.ip);

    // Open control socket
    open_connect_TCP_socket(&control_socketfd, params.ip, FTP_PORT);

    // print socket response
    print_socket_response(control_socketfd);

    // send command (username) and get response
    if((error_code = send_cmd_to_socket(control_socketfd, "user", params.username, response)) != 331){
        printf("Error Sending username command! error_code = %i\n", error_code);
        exit(-1);
    }

    // send command (password) and get response
    if((error_code = send_cmd_to_socket(control_socketfd, "pass", params.password, response)) != 230){
        printf("Error Sending password command! error_code = %i\n", error_code);
        exit(-1);
    }

    // send pasv command and get response
    if((error_code = send_cmd_to_socket(control_socketfd, "pasv", params.password, response)) != 227){
        printf("Error Sending pasv command! error_code = %i\n", error_code);
        exit(-1);
    }

    // get port from pasv command response
    if(parse_pasv_response(response, &port, params.ip) < 0){
        exit(-1);
    }
    // open data socket
    open_connect_TCP_socket(&data_socketfd, params.ip, port);

    // send command (retr) and get response
    if((error_code = send_cmd_to_socket(control_socketfd, "retr", params.url_path, response)) != 150){
        printf("Error Sending retr command! error_code = %i\n", error_code);
        exit(-1);
    }
    // get bytes sent to data packet
    parse_retr_response(response, &bytes_sent);

    // download file from data socket
    if((bytes_received = download_file(data_socketfd, params.filename)) == -1){
        printf("Failed to Download File: cannot open files!\n");
        exit(-1);
    } else if(bytes_received != bytes_sent){
        printf("Failed to Download File: number of bytes sent is different from bytes received!\n");
        printf("bytes sent: %i\n", bytes_sent);
        printf("bytes received: %i\n\n", bytes_received);
        exit(-1);
    } else{
        printf("bytes received: %i\n", bytes_received);
        printf("File Downloaded with success!\n");
    }

    // close control socket
    if (close(control_socketfd) < 0){
        perror("close()");
        exit(-1);
    }
    // close data socket
    if (close(data_socketfd) < 0){
        perror("close()");
        exit(-1);
    }

    return 0;
}
