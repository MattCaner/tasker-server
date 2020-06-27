#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <time.h>
#include <inttypes.h>

char PORT[8];
char SERV_ADDR[INET6_ADDRSTRLEN];
char CUSTOM_MSG[129];

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



//example input: ./sender.out test.out arg1
int main(int argc, char** argv){

    if(argc>=2){
    FILE* config;
    config = fopen(argv[1],"r");
    if(config){
        //read config:
        fscanf(config,"server=%s\nserver_port=%s\ncustom_msg=%s\n",SERV_ADDR,PORT,CUSTOM_MSG);
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
    printf("CONFIG VALUES:\nServer address: %s, port %s\n",SERV_ADDR,PORT);
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
        perror("");
        return -1;
    }
    if (connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
        close(sockfd);
        perror("Failed connecting");
        return -1;
    }

    char date_buffer[26];
    time_t t = time(NULL);
  	struct tm tm = *localtime(&t);
    strftime(date_buffer, 26,"%Y-%m-%d %H:%M:%S", &tm);

    int primal_payload_size = 128+129;
   
    char* buffer = malloc(sizeof(char)*(primal_payload_size));


    sprintf(buffer,"SSender was activated at %s. Message: %s",date_buffer,CUSTOM_MSG);
    int payload_size = strlen(buffer)+1;

    write(sockfd,buffer,payload_size);

    printf("MESSAGE: %s\n",buffer);
	printf("Notification sent to the server.\n");

    free(buffer);

	return 0;
}
