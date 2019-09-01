/*
 * schnapsen_client.c
 *
 *  Created on: 15.08.2019
 *      Author: michi
 */
#include<sys/socket.h>
#include<netinet/ip.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<errno.h>
#include<pthread.h>
#include<stdlib.h>
#include<stdio.h>
#include<poll.h>

#ifndef PORT
#define PORT 8011
#endif

#define READ_SIZE 64

int pass(int fd1,int fd2){
	char buf[READ_SIZE];
	ssize_t readv;
	do {
		readv = read(fd1, buf, READ_SIZE);
		if(readv<0){
			if(errno&EINTR){
				continue;
			} else {
				return -1;
			}
		}
		size_t remaining=(size_t)readv;
		do {
			ssize_t written = write(fd2, buf, remaining);
			if(written<0){
				if(errno&EINTR){
					continue;
				} else {
					return -1;
				}
			}
			remaining -= (size_t)written;
		} while(remaining);
	} while(readv);
	return 0;
}

__attribute__ ((flatten))
void* pass_thread(void* args){
	int ret = pass(((int*)args)[0],((int*)args)[1]);
	exit(ret);
}

int main(){
	int fds1[2];
	int fds2[2];
	{
		struct in_addr server_address;
		if(inet_pton(AF_INET,getenv("SCHNAPSEN_SERVER_ADDRESS"), &server_address)<1){
			perror("inet_pton");
			return -1;
		}
		int socket_fd=socket(AF_INET,SOCK_STREAM,0);
		struct sockaddr_in address = {AF_INET,htons(PORT),server_address,{}};
		if(connect(socket_fd,(struct sockaddr*)&address,INET_ADDRSTRLEN)){
			perror("connect");
			return -1;
		}
		fds1[0] = socket_fd;
		fds1[1] = 1;
		fds2[0] = 0;
		fds2[1] = socket_fd;
	}
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_t thread1;
	pthread_create(&thread1, &attr, pass_thread, fds1);
	pthread_exit(pass_thread(fds2));
}
