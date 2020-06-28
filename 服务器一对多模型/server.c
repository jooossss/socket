#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ERR_EXIT(m) \
	do{ \
		perror(m); \
		exit(0);\
	}while(0);

void do_server(int connfd);

int main(){
	int listenfd;
	if((listenfd = socket(PF_INET,SOCK_STREAM,0)) < 0)
		ERR_EXIT("socket faile");
	
	struct sockaddr_in servaddr;
	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(7788);
	servaddr.sin_addr.s_addr = htons(INADDR_ANY);

	int  on = 1;
	if( setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)) < 0)
		ERR_EXIT("setsockopt");

	if(bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr)) < 0)
		ERR_EXIT("bind");

	if( listen(listenfd,SOMAXCONN) < 0)
		ERR_EXIT("listen");

	struct sockaddr peeraddr;
	socklen_t peerlen = sizeof(struct sockaddr);

	while(1){
		struct sockaddr peeraddr;
		socklen_t peerlen = sizeof(struct sockaddr);
		int connfd;
		if((connfd = accept(listenfd,&peeraddr,&peerlen)) < 0)
			ERR_EXIT("accept");
		
		if(fork() == 0){ //chile
			close(listenfd);
			do_server(connfd);
			close(connfd);
			return 0;
		}
		else{ //parent
			close(connfd);
		}
	}
	return 0;
}

void do_server(int connfd){
	char recvbuf[1024] = {0};
	while(1){
		memset(recvbuf,0,sizeof(recvbuf));
		int ret = read(connfd,recvbuf,sizeof(recvbuf));
		if(ret == 0){
			printf("clien close\n");
			break;		
		}else if(ret == -1){
			ERR_EXIT("read");		
		}
		fputs(recvbuf,stdout);
		write(connfd,recvbuf,ret);
	}
}
