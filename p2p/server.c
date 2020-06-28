#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <signal.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define ERR_EXIT(m) \
	do{ \
		perror(m); \
		exit(0);\
	}while(0);

void handler(int sig){
	printf("recv a sig = %d\n",sig);
	exit(EXIT_SUCCESS);
}

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
	int connfd;
	if((connfd = accept(listenfd,&peeraddr,&peerlen)) < 0)
		ERR_EXIT("accept");
		
	if(fork() == 0){ //chile
		char recvbuf[1024] = {0};
		while(1){
			int ret = read(connfd,recvbuf,sizeof(recvbuf));
			if(ret == 0){
				printf("clien close\n");			
				break;
			}
			else if(ret == -1){
				ERR_EXIT("read");			
			}  
			else{
				fputs(recvbuf,stdout);
				memset(recvbuf,0,sizeof(recvbuf));
			}
		}
		kill(getppid(),SIGUSR1); //send a signal while chile process close,info the parent process close too;
		close(connfd); 		
	}
	else{ //parent
		signal(SIGUSR1,handler); // register signal,while recv a signal callback the handler;
 		char writebuf[1024] = {0};
		while(fgets(writebuf,sizeof(writebuf),stdin) != NULL){
			write(connfd,writebuf,strlen(writebuf));
			memset(writebuf,0,sizeof(writebuf));	
		}
		close(connfd);
	}
	return 0;
}


