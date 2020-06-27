#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

//global config:
char PORT [8] = "7777";
unsigned int connection_failed_policy = 3;
unsigned int max_listeners = 10;

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

int get_human_address(struct sockaddr* saddr, char* buffer){
    if(saddr->sa_family==AF_INET){
        inet_ntop(AF_INET,&(((struct sockaddr_in*)saddr)->sin_addr),buffer,INET_ADDRSTRLEN);
        return 0;
    }else if(saddr->sa_family==AF_INET6){
        inet_ntop(AF_INET6,&(((struct sockaddr_in6*)saddr)->sin6_addr),buffer,INET6_ADDRSTRLEN);
        return 0;
    } else{
        return -1;
    }
}

struct ListenerData{
    struct addrinfo* addr;
    int addrlen;
    int id;
    int n_fails;
    unsigned char market_to_delete;
};

int appendToList(struct ListenerData** list, unsigned int* listeners_size, struct ListenerData* toAppend){
    if(*listeners_size >= max_listeners) return -1;
    *listeners_size = *listeners_size+1;
    list[*listeners_size-1] = toAppend;
    return 0;
}


int removeClient(struct ListenerData** list, unsigned int* listeners_size, int index){
    if(listeners_size == 0) return -1;
    free(list[index]->addr);
    free(list[index]);
    for(int i = *listeners_size-2; i <= index; i++){
        list[i] = list[i+1];
    }
    *listeners_size = *listeners_size - 1;
    return 0;
}

struct ListenerData* createClient(struct addrinfo* addr, int addrlen, int id){
    struct ListenerData* l = malloc(sizeof(struct ListenerData));
    l->addr = malloc(sizeof(struct addrinfo));
    memcpy(l->addr,addr,addrlen);
    l->addrlen = addrlen;
    l->id = id;
    l->n_fails = 0;
    l->market_to_delete = 0;
    return l;
}


int main(int argc, char**argv){
    

    if(argc>=2){
        FILE* config;
        config = fopen(argv[1],"r");
        if(config){
            //read config:
            int pbuf;
            fscanf(config,"port=%d\nmax_connection_failures=%d\nmax_listeners=%d\n",&pbuf,&connection_failed_policy,&max_listeners);
            sprintf(PORT,"%d",pbuf);
        }else{
            perror("Incorrect config file");
            printf("Assuming default values\n");
        }
    }
    else{
        printf("No config file supplied, assuming default values\n");
    }
    printf("----------------------------------\n");
    printf("CONFIG VALUES:\nPORT: %s\nMaximum times a listener will be polled before removing it: %d\nMaximum listeners: %d\n",PORT,connection_failed_policy,max_listeners);
    printf("----------------------------------\n");

    unsigned int listeners_size = 0;
    struct ListenerData** listeners = malloc(sizeof(struct ListenerData*)*max_listeners);

	struct addrinfo addrSetup;
	memset(&addrSetup, 0, sizeof(addrSetup));
	addrSetup.ai_flags = AI_PASSIVE;
	addrSetup.ai_family = AF_UNSPEC;
	addrSetup.ai_socktype = SOCK_STREAM;

	struct addrinfo *servinfo;
	int addrstatus = getaddrinfo(0, PORT, &addrSetup, &servinfo);

	if(addrstatus != 0){
		perror("address error");
		return -1;
	}
	praddr(servinfo);
	
	int sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);

	if(sockfd < 0){
		perror("");
		return -1;
	}
	
	printf("Server socket is opened...\n");
	if(bind(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1){
		perror("");
		return -1;
	}
	printf("Listening at port %s...\n",PORT);
	
	if(listen(sockfd,10) == -1){
		perror("");
		return -1;
	}

	struct sockaddr client_info;
	unsigned int client_addrlen = sizeof(client_info);

    int newsockfd = 0;


    while(1){

        newsockfd = accept(sockfd,&client_info,&client_addrlen);
        if(newsockfd<0){
            perror("");
        }
        else{
            //read the first character
            printf("Message received!\n");
            char firstchar;
            read(newsockfd,&firstchar,1);

            switch(firstchar){
                //Sender's protocol:
                case 'S':
                    printf("Sender has spoken!\n");

                    int size = 1024;
                    char* buffer = malloc(sizeof(char)*size);

                    char c = 1;

                    for(int position = 0; c!='\0'; position++){
                        //expand buffer
                        if(position == size){
                            size += 1024;
                            buffer = realloc(buffer,sizeof(char)*size);
                        }
                        int r = read(newsockfd,&c,1);
                        if(r==0){
                            perror("error accepting bytes");
                        }
                        buffer[position] = c;
                    }
                    
                    printf("----------------------------------\n%s\n----------------------------------\n",buffer);

                    //try connect
                    printf("creating socket to connect...\n");

                    int lsockfd = socket( servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol );
                    if(lsockfd < 0) {
                        perror("Failed creating socket");
                        return -1;
                    }

                    int listeners_succ = 0;
                    int listeners_fail = 0;
                    int listeners_removed = 0;

                    for(int i = 0; i<listeners_size; i++){
                        if (connect(lsockfd, listeners[i]->addr->ai_addr, listeners[i]->addr->ai_addrlen) == -1) {
                            listeners_fail++;
                            close(lsockfd);
                            perror("Failed connecting to a listener");
                            //automatic unsubscribe:
                            listeners[i]->n_fails++;
                            if(listeners[i]->n_fails >= connection_failed_policy){
                                listeners[i]->market_to_delete = 1;
                                listeners_removed++;
                                printf("Listener marked to be removed due to surpassing policied amount of consecutive connection failures.\n");
                            }
                        }else{
                            write(lsockfd,buffer,strlen(buffer)+1);
                            listeners_succ++;
                            listeners[i]->n_fails = 0;
                        }
                    }
                    for(int i = listeners_size-1; i>=0; i--){
                        if(listeners[i]->market_to_delete==1){
                            removeClient(listeners,&listeners_size,i);
                        }
                    }
                    
                    printf("Notification sent to %d listeners\n%d listeners were reached, %d listeners were unreachable\n%d listeners were removed from the list due to recent consecutive unreachabilities\n",
                    listeners_succ+listeners_fail,listeners_succ,listeners_fail,listeners_removed);

                    free(buffer);

                break;

                //Hello protocol:
                case 'H':
                    ;
                    printf("Received Hello protocol\n");
                    //get port:
                    char portbuffer[10];
                    portbuffer[0] = 1;

                    char char_buf = 1;

                    for(int i = 0; char_buf != '\0'; i++){
                        int read_bytes = read(newsockfd,&char_buf,1);
                        if(read_bytes==0){
                            perror("Data came corrupted!");
                        }
                        portbuffer[i] = char_buf;
                    }

                    char address_buffer[INET6_ADDRSTRLEN];
                    get_human_address(&client_info,address_buffer);

                    struct addrinfo* listener_info;

                    printf("New listener at %s:%s\n",address_buffer,portbuffer);
                    int addrstatus = getaddrinfo(address_buffer, portbuffer, &addrSetup, &listener_info);
                    if(addrstatus<0){
                        perror("Error creating a listener address");
                    }
                    praddr(listener_info);

                    struct ListenerData* l = createClient(listener_info,sizeof(struct addrinfo),atoi(portbuffer));
                    if(appendToList(listeners,&listeners_size,l) == -1){
                        printf("To many listeners! New listener was not added.\n");
                    }

                break;
            }
        }
    }

    free(listeners);


	return 0;
}