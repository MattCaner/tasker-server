#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

char SERV_ADDR [INET6_ADDRSTRLEN];
char PORT [8];
char CLIENT_PORT [8];

void praddr( struct addrinfo* info ) {
    void* addr;
    if ( info->ai_family == AF_INET ){ 
        struct sockaddr_in *ip = (struct sockaddr_in*)info->ai_addr;
        addr = &(ip->sin_addr);
    } 
    else {
        struct sockaddr_in6 *ip = (struct sockaddr_in6 *)info->ai_addr;
        addr = &(ip->sin6_addr);
    }
    char human_address[INET6_ADDRSTRLEN];
    inet_ntop(info->ai_family, addr, human_address, INET6_ADDRSTRLEN);
}

int main(int argc, char** argv){

    if(argc>=2){
        FILE* config;
        config = fopen(argv[1],"r");
        if(config){
            //read config:
            fscanf(config,"port=%s\nserver=%s\nserver_port=%s\n",CLIENT_PORT,SERV_ADDR,PORT);
        }else{
            perror("Incorrect config file");
            return -1;
        }
    }
    else{
        printf("No config was given.\n");
        return -1;
    }
    printf("----------------------------------\n");
    printf("CONFIG VALUES:\nPORT: %s\nServer address: %s, port %s\n",CLIENT_PORT,SERV_ADDR,PORT);
    printf("----------------------------------\n");

	struct addrinfo addrSetup;
	memset(&addrSetup, 0, sizeof(addrSetup));
	addrSetup.ai_flags = AI_PASSIVE;
	addrSetup.ai_family = AF_UNSPEC;
	addrSetup.ai_socktype = SOCK_STREAM;

	struct addrinfo* servinfo;
	int addrstatus = getaddrinfo(SERV_ADDR, PORT, &addrSetup, &servinfo);

	if(addrstatus != 0){
		perror("address error");
		return -1;
	}
	praddr(servinfo);

    int sockfd = socket( servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol );
    if(sockfd < 0) {
        perror("Failed creating socket");
        return -1;
    }
    if (connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
        close(sockfd);
        perror("Failed connecting");
        return -1;
    }

    char c[8];
    int msglen = 8;
    sprintf(c,"H%s",CLIENT_PORT);
    printf("Sending Hello protocol\n");
    write(sockfd,c,msglen);

    //create client socket to lisna'

    struct addrinfo* client_info;
    int cli_status = getaddrinfo("127.0.1",CLIENT_PORT,&addrSetup,&client_info);

	if(cli_status != 0){
		perror("client address error");
		return -1;
	}

    int cli_socket = socket(client_info->ai_family,client_info->ai_socktype,client_info->ai_protocol);
    if(cli_socket<0){
        perror("Failed creating client socket");
        return -1;
    }

    int clistatus = bind(cli_socket,client_info->ai_addr,client_info->ai_addrlen);

    if(clistatus<0){
        perror("Bind error");
        return -1;
    }


    if(listen(cli_socket,1) == -1){
        perror("Failed changing mode to listen");
    }

    printf("Succesfully connected.\n");

    //listen
    while(1){
        struct sockaddr server_info;
        unsigned int server_addrlen = sizeof(server_info);
        int newsockfd = accept(cli_socket,&server_info,&server_addrlen);

        if(newsockfd<0){
            perror("error accepting");
            return -1;
        }
        int size = 1024;
        char* buffer = malloc(sizeof(char)*size);

        char c = 1;
        printf("----------------------------------\n----------------------------------\n");
        printf("NOTIFICATION RECEIVED FROM THE SERVER!!!\n");
        for(int position = 0; c!='\0'; position++){

            if(read(newsockfd,&c,1) == 0){
                printf("0 bytes were read from input! This is an error.\n");
                return -1;
            }
            
            //expand buffer
            if(position == size){
                size += 1024;
                buffer = realloc(buffer,sizeof(char)*size);
            }
            buffer[position] = c;
        }
        
        printf("----------------------------------\n%s\n----------------------------------\n----------------------------------\n",buffer);

        free(buffer);
    }
	return 0;
}
