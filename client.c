/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define TRUE   1
#define FALSE  0
#define ERROR -1

#define PORT "3500" // the port client will be connecting to 
#define MAXDATASIZE 1000 // max number of bytes we can get at once 

typedef struct{
	char field[MAXDATASIZE];
	int numbytes;
}Message;

void getcmd(Message *cmd, Message *name);
int validcmd(Message *cmd);
int exten(Message *cmd);

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	int sockfd, numbytes;  
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
	Message cmd;
	Message name;
	Message input;

	if (argc != 2) {
	    fprintf(stderr,"usage: client hostname\n");
	    exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure
	
	do{
		getcmd(&cmd, &name);
		send(sockfd, cmd.field, cmd.numbytes, 0);
		if(exten(&cmd)){
			sleep(1);
			send(sockfd, name.field, name.numbytes, 0);
		}if(!strcmp(cmd.field, "list")){
			input.numbytes=recv(sockfd, input.field, MAXDATASIZE-1, 0);
			input.field[input.numbytes]='\0';
			printf("%s", input.field);
		}else if(!strcmp(cmd.field, "check")){
			input.numbytes=recv(sockfd, input.field, MAXDATASIZE-1, 0);
			input.field[input.numbytes]='\0';
			printf("%s", input.field);
		}else if(!strcmp(cmd.field, "display")){
			do{
			input.numbytes=recv(sockfd, input.field, MAXDATASIZE, 0);
			if(input.numbytes!=MAXDATASIZE) input.field[input.numbytes]='\0';
			printf("%s", input.field);
			}while(input.numbytes==MAXDATASIZE);
		}else if(!strcmp(cmd.field, "download")){
			input.numbytes=recv(sockfd, input.field, MAXDATASIZE-1, 0);
			input.field[input.numbytes]='\0';
			printf("%s", input.field);
		}else if(!strcmp(cmd.field, "help")){
			input.numbytes=recv(sockfd, input.field, MAXDATASIZE-1,0);
			input.field[input.numbytes]='\0';
			printf("%s", input.field);
		}
	}while(strcmp(cmd.field,"quit"));
	close(sockfd);
	return 0;
}

void getcmd(Message *cmd, Message *name){
	int i, j, event;
	do{
		i=0;
		j=0;
		event=FALSE;
		printf("client367: ");
		while((cmd->field[i]=getchar())!='\n'&&cmd->field[i]!=' '){
			i++;
		}if(cmd->field[i]==' '){
			while((name->field[j]=getchar())!='\n'&&name->field[j]!=' '){
				j++;
			}if(name->field[j]==' ') while(getchar()!='\n'){}
		}cmd->field[i]='\0';
		name->field[j]='\0';
		if(exten(cmd)){
			if(j==0){
				event=TRUE;
				printf("missing parameter\n");
			}
		}
	}while(!validcmd(cmd)||event);
	cmd->numbytes=i;
	name->numbytes=j;
	return;
}

int validcmd(Message *cmd){
	int i;
	i=!strcmp(cmd->field, "list");
	i=i+!strcmp(cmd->field, "check");
	i=i+!strcmp(cmd->field, "display");
	i=i+!strcmp(cmd->field, "download");
	i=i+!strcmp(cmd->field, "quit");
	i=i+!strcmp(cmd->field, "help");
	if(i!=TRUE) printf("Invalid Command\n");
	return i;
}

int exten(Message *cmd){
	int i;
	i=!strcmp(cmd->field, "check");
	i=i+!strcmp(cmd->field, "display");
	i=i+!strcmp(cmd->field, "download");
	return i;
}
