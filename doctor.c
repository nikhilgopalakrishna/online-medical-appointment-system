/*
 * Author	:	Nikhil Gopalakrishna (ngopalak@usc.edu)
 * File		:	doctor.c
 * Description 	:	creates two doctor datagram socket servers.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include "doctor.h"
#define PORT1 "41720" // Doctor1,  port number, the UDP port to which patients will be connecting to
#define PORT2 "42720" // Doctor2   port number, the UDP port to which patients will be connecting to
#define BACKLOG 10 // how many pending connections queue will hold
#define MAXDATASIZE 100

void sigchld_handler(int s)
{
	while(waitpid(-1, NULL, WNOHANG) > 0);
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
	return &(((struct sockaddr_in*)sa)->sin_addr);
}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void create_server(const char*  portnumber, struct addrinfo *servinfo)
{

   struct sigaction sa;
   int sockfd;
   socklen_t len;
   struct sockaddr_in servaddr,cliaddr;
   char buf[MAXDATASIZE];
   doc_insurance_info_t  data[3];
   FILE *file ;

   //Read insurance plans for each doctor
    if(!strcmp(portnumber,PORT1)) {
        file = fopen("doc1.txt","r");
    }
    else{
        file = fopen("doc2.txt","r");
    }

    if(file == NULL) {
    perror("Error opening file");
    exit(0);
    }
    int i =0;
    char *start_p, *tab_p;

    while(fgets(buf, sizeof(buf), file) !=NULL){
       start_p = strtok(buf,"\n");
       tab_p = strchr(start_p, ' ');
       *tab_p++ = '\0';
       strcpy(data[i].insurance , start_p);
       strcpy(data[i].cost, tab_p);
       i++;
    }
    fclose(file);

  // create socket  
  if((sockfd=socket(AF_INET,SOCK_DGRAM,0))== -1) {
	perror("server: socket");
	exit(1);
    }

   struct hostent *h_ptr = gethostbyname("localhost");

   memset(&servaddr,0,sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_port=htons(atoi(portnumber));
   memcpy((void*)&servaddr.sin_addr, h_ptr->h_addr_list[0], h_ptr->h_length);

   // bind socket to localhost
   if((bind(sockfd,(struct sockaddr *)&servaddr,sizeof(servaddr)))== -1) {
	close(sockfd);
	perror("server: bind");
	exit(1);
    }

        struct sockaddr_in localAddress;
        socklen_t addressLength = sizeof(localAddress);
	//Retrieve  the  locally bound  name  of  the  specified  socket  and  store  it  in  the  sockaddr structure
        if((getsockname(sockfd, (struct sockaddr*)&localAddress,  &addressLength)) ==1){
        perror("getsockname");
        exit(1);
        }

        if(!strcmp(portnumber,PORT1)){
      printf("Phase 3: Doctor 1 has a static UDP port %d and IP address %s.\n",  (int) ntohs(localAddress.sin_port),inet_ntoa(servaddr.sin_addr));
    }
    else{
        printf("Phase 3: Doctor 2 has a static UDP port %d and IP address %s.\n",  (int) ntohs(localAddress.sin_port),inet_ntoa(servaddr.sin_addr));
    }

int numbytes = 0;
len = sizeof(cliaddr);
 
    while(1)
    {
        // Receive insurance information of patient   
        if ((numbytes =  recvfrom(sockfd,buf,MAXDATASIZE,0,(struct sockaddr *)&cliaddr,&len)) == -1) {
        perror("recvfrom");
        close(sockfd);
        exit(1);
    }

    struct sockaddr_in *sin = (struct sockaddr_in *)&cliaddr;

    if(!strcmp(portnumber,PORT1)) {
        printf("Phase 3: Doctor 1 receives the request from the patient with port number %d and the insurance plan %s.\n",ntohs(sin->sin_port),buf);
    }
    else{
        printf("Phase 3: Doctor 2 receives the request from the patient with port number %d and the insurance plan %s.\n",ntohs(sin->sin_port),buf);
    }

for(i=0;i<3;i++){

    if(!strcmp(buf,data[i].insurance))
    {
            memset(buf, 0, sizeof(buf));
            strcpy(buf, data[i].cost);

             if(!strcmp(portnumber,PORT1)) {
                printf("Phase 3: Doctor 1 has sent estimated price %s$ to patient with port number %d.\n", data[i].cost, ntohs(sin->sin_port));
             }
             else{
                printf("Phase 3: Doctor 2 has sent estimated price %s$ to patient with port number %d.\n", data[i].cost, ntohs(sin->sin_port));
             }
             // Send cost estimation to patient
             if((numbytes=sendto(sockfd,buf,sizeof(buf),0,(struct sockaddr *)&cliaddr,sizeof(cliaddr))) == -1){
                perror("sendto");
                close(sockfd);
                exit(0);
    }
    }
}
}
close(sockfd);

// reap all dead processes
sa.sa_handler = sigchld_handler;
sigemptyset(&sa.sa_mask);
sa.sa_flags = SA_RESTART;

if (sigaction(SIGCHLD, &sa, NULL) == -1) {
	perror("sigaction");
	exit(1);
}

}

int main(void)
{

	int pid;
        //Fork a doctor in the child process and another in parent process  
	if((pid = fork()) == 0)
	{
          // Create UDP socket server for Doctor1
	  struct addrinfo  *servinfo1 = NULL;
	  create_server(PORT1, servinfo1);

	}
	  //Create UDP socket server for Doctor2
          struct addrinfo  *servinfo2 = NULL;
          create_server(PORT2, servinfo2);

	return 0;
}
