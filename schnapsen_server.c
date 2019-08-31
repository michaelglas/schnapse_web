/*
 * schnapsen_server.c
 *
 *  Created on: 14.08.2019
 *      Author: michi
 */

#define _GNU_SOURCE
#include<netinet/ip.h>
#include<unistd.h>
#include<stdio.h>
#include<poll.h>
#include<errno.h>
#include<stdlib.h>
#include<signal.h>
#include<string.h>

#define MAX_QUEUE_LENGTH 10

const char* sdefault(const char* a,const char* d){
	if(a)
		return a;
	else
		return d;
}

int main(){
	int _socket_fd=socket(AF_INET, SOCK_STREAM|SOCK_NONBLOCK|SOCK_CLOEXEC, 0);
	if(_socket_fd<0){
		perror("socket");
		return -1;
	}
	struct sockaddr_in _address = {AF_INET,8011,{htonl(INADDR_LOOPBACK)},0};
	if(bind(_socket_fd,(struct sockaddr*)&_address,INET_ADDRSTRLEN)==-1){
		perror("bind");
		return -1;
	}
	if(listen(_socket_fd,MAX_QUEUE_LENGTH)==-1){
		perror("listen");
		return -1;
	}
	struct pollfd poll_fds[2]={{_socket_fd,POLLIN,0},{-1,POLLRDHUP,0}}; // @suppress("Symbol is not resolved")
	int last_fd=-1;
	while(1){
		if(poll(poll_fds,1+(last_fd>0),-1)>0){
			if(poll_fds[0].revents&POLLIN){
				if(last_fd>0){
					int socket_fd2 = accept(_socket_fd,NULL,NULL);
					if(socket_fd2<0){
						perror("accept");
						continue;
					}
					pid_t fork_ret=fork();
					if(fork_ret<0){
						perror("fork");
						close(socket_fd2);
						continue;
					} else if(fork_ret==0){
						char buf1[20];
						snprintf(buf1, 20, "%d",last_fd);
						char buf2[20];
						snprintf(buf2, 20, "%d",socket_fd2);
						char *const a[4]={"schnapsen_fds",buf1,buf2,NULL};
						execv(sdefault(getenv("SCHNAPSEN_FDS_EXE"), "../schnapsen_fds"), a);
						perror("execv");
						kill(getppid(), SIGTERM);
						return -1;
					} else {
						close(last_fd);
						close(socket_fd2);
						last_fd=-1;
					}
				} else {
					last_fd = accept(_socket_fd,NULL,NULL);
					poll_fds[1].fd = last_fd;
				}
			} else if(poll_fds[0].revents&(POLLNVAL|POLLERR)) {
				close(_socket_fd);
				return -1;
			}
			if(last_fd>0&&poll_fds[1].revents){
				close(last_fd);
				last_fd = -1;
			}
		} else {
			sleep(0.01);
		}
	}
}
