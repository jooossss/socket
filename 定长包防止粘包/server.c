#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#define ERR_EXIT(m) \
	do{ \
		perror(m); \
		exit(0);\
	}while(0);

struct packet{
	int len;
	char buf[1024];
};

void do_server(int connfd);
ssize_t nread(int fd, void *buf, size_t count);
ssize_t nwrite(int fd, const void *buf, size_t count);
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
	struct packet recvbuf;
	while(1){
		memset(&recvbuf,0,sizeof(recvbuf));
		int ret = nread(connfd,&recvbuf.len,4);
		if(ret == -1){
			ERR_EXIT("read");		
		}else if(ret < 4){
			printf("clien close\n");
			break;	
		}
		
		int n = ntohl(recvbuf.len);
		ret = nread(connfd,recvbuf.buf,n);
		if(ret == -1){
			ERR_EXIT("read");		
		}else if(ret < n){
			printf("clien close\n");
			break;	
		}
		

		fputs(recvbuf.buf,stdout);
		nwrite(connfd,&recvbuf,n+4);
	}
}


ssize_t nread(int fd, void *buf, size_t count){
	
	size_t nleft = count;
	size_t nread;
	char* bufp = (char*)buf;
	while(nleft > 0){	
		nread = read(fd,bufp,nleft);
		if(nread == 0){  //eof
			return count - nleft;
		}
		else if(nread < 0){
			if(errno == EINTR) continue;   //intr
			else return -1;			//error
		}
		
		bufp += nread;
		nleft -= nread;
	}
	return count;
}



ssize_t nwrite(int fd, const void *buf, size_t count){
	size_t nleft = count;
	size_t nwrite;
	char* bufp = (char*)buf;
	while(nleft > 0){	
		nwrite = write(fd,bufp,nleft);
		if(nwrite == 0){  //eof
			continue;
		}
		else if(nwrite < 0){
			if(errno == EINTR) continue;   //intr
			else return -1;			//error
		}
		
		bufp += nwrite;
		nleft -= nwrite;
	}
	return count;
}











