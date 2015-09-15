/*
 * Author	:	Nikhil Gopalakrishna (ngopalak@usc.edu)
 * File		:	healthcenterserver.c
 * Description 	:	create a stream socket server.
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
#include <pthread.h>
#include "healthcenterserver.h"
#define PORT "21720" // the port users will be connecting to
#define BACKLOG 10 // how many pending connections queue will hold
#define MAXDATASIZE 100


char users[2][MAXDATASIZE];
availabilityinfo_t availabilityinfo[6];

/* code template as used in www.beej.us/guide/bgnet/  */

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

/* code template as used in www.beej.us/guide/bgnet/  */

// HealthCenterServer Handler
void *server_handler(void *server_args)
{

     int i=0;
     char buf[MAXDATASIZE];
     char s[INET6_ADDRSTRLEN];
     char *start_p, *tab_p;
     int new_fd = ((server_handler_args_t*)server_args)->new_fd;
     int client_port = ((server_handler_args_t*)server_args)->client_port;
     strcpy(s, ((server_handler_args_t*)server_args)->s);

    int numbytes;
    memset(buf, 0, sizeof(buf));

    //Receive authentication information from patient
    if((numbytes = recv(new_fd, buf, MAXDATASIZE, 0)) == -1) {
     perror("recv");
     exit(1);
     }

    i=0;
    int authentication_success=0;
    start_p = strchr(buf, ' ');
    start_p++;

     char buf1[MAXDATASIZE];
     memset(buf1, 0, sizeof(buf1));
     strcpy(buf1,start_p);
     tab_p = strchr(buf1, ' ');
     *tab_p++ = '\0';
     tab_p = strtok(tab_p,"\n");

     printf("Phase 1: The Health Center Server has received request from a patient with username %s and password %s.\n", buf1, tab_p);

     while(i<2){
        if(strcmp(users[i],start_p)){
            i++;
        }
        else{
       authentication_success =1;
        break;
        }
     }
     
     //Send autentication response to patient 
     if(authentication_success){

        printf("Phase 1: The Health Center Server sends the response success to the patient with username %s.\n",  buf1);
         if (send(new_fd, "success", 20, 0) == -1){
        perror("send");
        close(new_fd);
        exit(0);
        }
	}
	else{

	printf("Phase 1: The Health Center Server sends the response failure to patient with username %s.\n",  buf1);
        if (send(new_fd, "failure", 20, 0) == -1){
        perror("send");
        close(new_fd);
        exit(0);
        }
        return NULL;
	}
        //Receive available from patient 
	if ((numbytes = recv(new_fd, buf, MAXDATASIZE, 0)) == -1) {
        perror("recv");
        exit(1);
     }

     printf("Phase 2: The Health Center Server, receives a request for available time slots from patients with port number %d and IP address %s.\n",client_port, s);

     int flag=0;
     memset(buf, 0, sizeof(buf));
     for(i=0;i<6; i++){

         if(!availabilityinfo[i].reserved){
                flag =1;
                 strcat(buf, availabilityinfo[i].availabily_info);
                 strcat(buf, " ");
            }
         }
        
         if(!flag){
                memset(buf, 0, sizeof(buf));
                strcpy(buf, "notavailable");
               if(send(new_fd, buf, sizeof(buf), 0) == -1){
                perror("send");
                close(new_fd);
                exit(0);
         }
               close(new_fd);
               return NULL;
          }

          printf("Phase 2: The Health Center Server sends available time slots to patient with username %s.\n", buf1);
          // Send availability information to patient
          if (send(new_fd, buf, sizeof(buf), 0) == -1){
                perror("send");
                close(new_fd);
                exit(0);
         }
            // Receive selection from patient 
            if ((numbytes = recv(new_fd, buf, sizeof(buf), 0)) == -1) {
            perror("recv");
            exit(1);
            }

            start_p = strchr(buf, ' ');
            int index = atoi(++start_p);

             if(!index){
                 close(new_fd);
                 pthread_exit(0);
             }

            printf("Phase 2: The Health Center Server receives a request for appointment %s from patient with port number %d and username %s.\n", availabilityinfo[index-1].availabily_info,client_port,buf1);

          if(!availabilityinfo[index-1].reserved){
           availabilityinfo[index-1].reserved = 1;
             memset(buf, 0, sizeof(buf));
             strcpy(buf, availabilityinfo[index-1].doctor_port);

               printf("Phase 2: The Health Center Server confirms the following appointment %s to patient with username %s.\n", availabilityinfo[index-1].availabily_info, buf1);
               // Send appointment information to patient
               if(send(new_fd, buf, sizeof(buf), 0) == -1){
                perror("send");
                close(new_fd);
                exit(0);
         }
        }
        else{
                memset(buf, 0, sizeof(buf));
                strcpy(buf, "notavailable");

               printf("Phase 2: The Health Center Server rejects the following appointment %d to patient with username %s.\n",index, buf1);
               // Send not available to patient if selection from patient is reserved
               if(send(new_fd, buf, sizeof(buf), 0) == -1){
                perror("send");
                close(new_fd);
                return NULL;
         }
        }

	close(new_fd);

	return NULL;
}


int main(void)
{

    int i=0;
    char buf[MAXDATASIZE];
    char *start_p, *tab_p;
    struct sockaddr_storage their_addr;// connector's address information
    struct sockaddr_in *sin_p;
    char s[INET6_ADDRSTRLEN];

	int sockfd, new_fd; // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;

	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;

	int rv;
    char host[100];
    char service[20];


    // Read usernames and passwords of patients from users.txt
    FILE *file = fopen("users.txt","r");
    if(file == NULL) {
    perror("Error opening file");
    return(-1);
    }

    while(fgets(buf, sizeof(buf), file) !=NULL){
        start_p= strtok(buf, "\n");
        strcpy(users[i] , start_p);
        i++;
    }
    fclose(file);

    memset(buf, 0, sizeof(buf));
    i=0;

    // Read availability information from availabilities.txt file 
    file = fopen("availabilities.txt","r");
    if(file == NULL) {
    perror("Error opening file");
    return(-1);
    }

    while(fgets(buf, sizeof(buf), file) !=NULL){
    start_p= strtok(buf, "\n");
    tab_p =  strchr(start_p, ' ');
    tab_p++;
    tab_p =  strchr(tab_p, ' ');
    tab_p++;
    tab_p =  strchr(tab_p, ' ');
    *tab_p++ = '\0';
    strcpy(availabilityinfo[i].availabily_info , start_p);
    strcpy(availabilityinfo[i].doctor_port , tab_p);
    availabilityinfo[i].reserved = 0;
    i++;
    }
    fclose(file);


/* code template as used in www.beej.us/guide/bgnet/  */
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo("localhost", PORT, &hints, &servinfo)) != 0) {
	fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
	return 1;
}

// loop through all the results and bind to the first we can
for(p = servinfo; p != NULL; p = p->ai_next) {
        //create socket
	if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) {
	perror("server: socket");
	continue;
}

if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1) {
	perror("setsockopt");
	exit(1);
}
//bind socket to localhost  
if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
	close(sockfd);
	perror("server: bind");
	continue;
}

        getnameinfo(p->ai_addr, p->ai_addrlen, host, sizeof(host), service, sizeof(service), 0);
        inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), host, sizeof(host));
	printf("Phase 1: The Health Center Server has port number %s and IP address %s\n", service, host);
	break;
}

if (p == NULL) {
	fprintf(stderr, "server: failed to bind\n");
	return 2;
}

freeaddrinfo(servinfo); // all done with this structure

	if (listen(sockfd, BACKLOG) == -1) {
	perror("listen");
	exit(1);
}

sa.sa_handler = sigchld_handler; // reap all dead processes
sigemptyset(&sa.sa_mask);
sa.sa_flags = SA_RESTART;

if (sigaction(SIGCHLD, &sa, NULL) == -1) {
	perror("sigaction");
	exit(1);
}
/* code template as used in www.beej.us/guide/bgnet/  */

 while(1) {

       // main accept() loop

	sin_size = sizeof their_addr;
	new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);

	if (new_fd == -1) {
	perror("accept");
	continue;
	}


     inet_ntop(their_addr.ss_family,get_in_addr((struct sockaddr *)&their_addr),s, sizeof s);
     sin_p = (struct sockaddr_in *)&their_addr;
     int  client_port = ntohs(sin_p->sin_port);

     pthread_t server_thread;

     server_handler_args_t *server_args = (server_handler_args_t*)malloc(sizeof(server_handler_args_t));

     server_args->client_port = client_port;
     server_args->new_fd = new_fd;
     strcpy(server_args->s,s);
     // Create server threads for each connections from the clients using multithreading
     if( pthread_create( &server_thread , NULL ,  server_handler , (void*) server_args) < 0)
     {
          perror("Server thread could not be created");
          close(new_fd);
          return 1;
     }

  }
  	return 0;
}
