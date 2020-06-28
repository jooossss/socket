#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#define ERR_EXIT(m) \
	do{ \
		perror(m); \
		exit(1);\
	}while(0)\


int main(){
	int conn;
	if((conn = socket(PF_INET,SOCK_STREAM,0)) < 0)
		ERR_EXIT("socket faile");
	
	struct sockaddr_in servaddr;
	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(7788);
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if(connect(conn,(struct sockaddr*)&servaddr,sizeof(servaddr)) < 0)
		ERR_EXIT("connect faile");
	char recvbuf[1024] = {0};
	char writebuf[1024] = {0};

	while(fgets(writebuf,sizeof(writebuf),stdin) != NULL){
		write(conn,writebuf,strlen(writebuf));
		read(conn,recvbuf,sizeof(recvbuf));
		fputs(recvbuf,stdout);
		memset(writebuf,0,1024);
		memset(recvbuf,0,1024);
		
	}
	shutdown(conn,SHUT_RDWR);
	close(conn);
	return 0;
}
