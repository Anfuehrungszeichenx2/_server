/*
** server.c -- first implemetation for server, mostly stolen from beji
** Jakob Hermanowski
*/


#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#include "build_return.h"
#define PORT "2222"

#define BACKLOG 100

void *get_in_addr(struct sockaddr *sa)
{
	if(sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void sigchld_handler(int s){
	(void)s;
	int saved_errno = errno;
	pid_t pid;
	while(pid = waitpid(-1, NULL, WNOHANG) > 0) {
		char buf[64];
		int len = snprintf(buf, sizeof(buf), "Reaped child process %d\n", pid);
    	write(STDOUT_FILENO, buf, len);
	}

	errno = saved_errno;
}

int main(void)
{
	int sockfd, new_fd;		//socket file descriptor
	struct addrinfo hints;
	struct addrinfo *servinfo; //pointer to the results
	struct addrinfo *p;

	struct sockaddr_storage in_addr;
	socklen_t sin_size;
	struct sigaction sa;	
	int yes=1;	//for the socket already in use error
	char s[INET6_ADDRSTRLEN];
	char ipstr[INET6_ADDRSTRLEN];
	int status; 	//return value

	memset(&hints, 0, sizeof hints);	//set memory to 0ros
	hints.ai_family = AF_UNSPEC;		//we accept ipv4 and 6	
	hints.ai_socktype = SOCK_STREAM;	//tcp socket stream
	hints.ai_flags = AI_PASSIVE;		//fill my ip for me
										
	//servinfo now points to a linked list typ addrinfo
	if ((status = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) { 
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		return 1;
	}

	//loop through all results and bind the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {

		if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
			perror("server: socket");
			continue;
		}
			//here we allow the socket to reuse the port so we dont get the already in use error
		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}
		
		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		} else {
			break;
		}
	}

	freeaddrinfo(servinfo); // we dont need that struct anymore

	if (p == NULL) {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {		// socket is ready to listen
		perror("listen");
		exit(1);
	}
	//Immer wenn ich ein SIGCHLD bekomme, ruf bitte meine Funktion
	//we might create dead processes with this we remove them	
	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	printf("server: waiting for connections...\n");

	while(1) {		// accept loop
	//printf("%d\n", search(test1_s, search_s));
		sin_size = sizeof in_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&in_addr, &sin_size); //blocking till we get input
		if (new_fd == -1) {
			perror("accept");
			exit(1);
			continue;
		}
		
		inet_ntop(in_addr.ss_family, get_in_addr((struct sockaddr *)&in_addr), s, sizeof s);
		printf("server: got connection from %s\n", ipstr);
		char content_len[38] = "Content-Type: text/html; charset=utf-8";	
		if (!fork()) { // here we fork the code, so we have one thread or smth running the parent and one the child. we check that with the ! if we are the parent we get the child id, so we dont enter, when we are the child we get a 0
					   
			close(sockfd); // we dont need that in the child

			uint16_t return_len;
			uint8_t *response;	
			build_return(&response, &return_len);
		    fprintf(stderr,"Antwort (%d Zeichen):\n%s\n", return_len, response);
			if (send(new_fd, response, return_len,0) == -1)
				perror("send");				
			free(response);
		//	if (send(new_fd, "HTTP/1.1 200 MEOW\nContent-Type: text/html; charset=utf-8\r\n\r\nüwü" , 65, 0) == -1)
		//		perror("send");
			close(new_fd);
			exit(0);
		}
		close(new_fd);
	}
	return 0;
}

