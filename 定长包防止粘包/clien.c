#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#define ERR_EXIT(m) \
	do{ \
		perror(m); \
		exit(1);\
	}while(0);

struct packet{
	int len;
	char buf[1024];
};
ssize_t nread(int fd, void *buf, size_t count);
ssize_t nwrite(int fd, const void *buf, size_t count);
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

	struct packet recvbuf;
	struct packet writebuf;
	memset(&recvbuf,0,sizeof(recvbuf));
	memset(&writebuf,0,sizeof(writebuf));

	while(fgets(writebuf.buf,sizeof(writebuf.buf),stdin) != NULL){

		int n = strlen(writebuf.buf);
		writebuf.len = htonl(n);
		nwrite(conn,&writebuf,n+4);
		
		int ret = nread(conn,&recvbuf.len,4);
		if(ret == -1){
			ERR_EXIT("read");		
		}else if(ret < 4){
			printf("clien close\n");
			break;	
		}
		
		n = ntohl(recvbuf.len);
		ret = nread(conn,recvbuf.buf,n);
		if(ret == -1){
			ERR_EXIT("read");		
		}else if(ret < n){
			printf("clien close\n");
			break;	
		}

		fputs(recvbuf.buf,stdout);

		memset(&recvbuf,0,sizeof(recvbuf));
		memset(&writebuf,0,sizeof(writebuf));
		
	}
	shutdown(conn,SHUT_RDWR);
	close(conn);
	return 0;
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
