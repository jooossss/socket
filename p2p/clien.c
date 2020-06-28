#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <signal.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#define ERR_EXIT(m) \
	do{ \
		perror(m); \
		exit(1);\
	}while(0)\

void handler(int sig){
	printf("recv a sig = %d\n",sig);
	exit(EXIT_SUCCESS);
}
int main(){
	int conn;
	if((conn = socket(PF_INET,SOCK_STREAM,0)) < 0)
		ERR_EXIT("socket faile");
	
	struct sockaddr_in servaddr;
	memset(&servaddr,0,sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(7788);
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	int on = 1;
	if( setsockopt(conn,SOL_SOCKET,SO_REUSEADDR,&on,sizeof(on)) < 0)
		ERR_EXIT("setsockopt");	

	if(connect(conn,(struct sockaddr*)&servaddr,sizeof(servaddr)) < 0)
		ERR_EXIT("connect faile");
	if(fork() == 0){

		char recvbuf[1024] = {0};
		while(1){
			int ret = read(conn,recvbuf,sizeof(recvbuf));
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
		kill(getppid(),SIGUSR1);
		close(conn);
	}
	else {
		signal(SIGUSR1,handler);
		char writebuf[1024] = {0};
		while(fgets(writebuf,sizeof(writebuf),stdin) != NULL){
			write(conn,writebuf,strlen(writebuf));
			memset(writebuf,0,1024);
		}
		close(conn);
	}
	shutdown(conn,SHUT_RDWR);
	close(conn);
	return 0;
}
