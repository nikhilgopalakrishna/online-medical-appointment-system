/*
 * Author	:	Nikhil Gopalakrishna (ngopalak@usc.edu)
 * File		:	patient2.c
 * Description 	:	creates patient2 client
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "patient2.h"

#define PORT "21720" // the port client will be connecting to
#define MAXDATASIZE 100 // max number of bytes we can get at once

//getavailable options from the availabilities sent by server 
void getavailableoptions(char* buf, char* options)
{

              char* p = buf;
              int i =3;
              char* start_p = strchr(buf, ' ');

              while(p != NULL){
                *start_p = '\0';
                if( i == 3){
                    strcat(options, p);
                    i=0;
                }
                i++;
                if(*(start_p+1) != '\0'){
                    p = ++start_p;
                    start_p = strchr(p, ' ');
                }
                else{
                    break;
                }
         }
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in*)sa)->sin_addr);
}
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    struct sockaddr_in localAddress;
    struct sockaddr_in servaddr;
    socklen_t addressLength = sizeof(localAddress);

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo("localhost", PORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
    }

    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
    //create socket
    if ((sockfd = socket(p->ai_family, p->ai_socktype,p->ai_protocol)) == -1) {
    perror("client: socket");
    continue;
    }
    // connect socket to server
    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
    close(sockfd);
    perror("client: connect");
    continue;

}

	//Retrieve  the  locally bound  name  of  the  specified  socket  and  store  it  in  the  sockaddr structure
        int getsock_check = getsockname(sockfd, (struct sockaddr*)&localAddress,  &addressLength);
       	//Error  checking
        if  (getsock_check==  1)  {
        perror("getsockname");
        exit(1);
        }

        printf("Phase 1: The Patient2 has TCP port number %d and IP address %s.\n", (int) ntohs(localAddress.sin_port), inet_ntoa( localAddress.sin_addr));
        break;
}

if (p == NULL) {
    fprintf(stderr, "client: failed to connect\n");
    return 2;
}

inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),s, sizeof s);

freeaddrinfo(servinfo); // all done with this structure

patientinfo_t *patient1 = (patientinfo_t*)malloc(sizeof(patientinfo_t));

    //Read patient2 username and password to file. 
    FILE *file  = fopen("patient2.txt", "r");
    if(file == NULL) {
    perror("Error opening file");
    return(-1);
    }

    fgets(buf, sizeof(buf), file);

    char *start_p= strtok(buf, "\n");
    char  *tab_p = strchr(start_p, ' ');
    *tab_p++ = '\0';

    strcpy(patient1->username , start_p);
    strcpy(patient1->password , tab_p);
    fclose(file);

    file  = fopen("patient2insurance.txt", "r");
    if(file == NULL) {
     perror("Error opening file");
     return(-1);
    }

    fgets(buf, sizeof(buf), file);
    start_p= strtok(buf, "\n");
    strcpy(patient1->insurance , start_p);
    fclose(file);

    strcpy(buf,"authenticate ");
    strcat(buf,patient1->username);
    strcat(buf, " ");
    strcat(buf, patient1->password);

    // Send authenticate request to healthcenter server
    if (send(sockfd, buf, sizeof(buf), 0) == -1){
            perror("send");
            close(sockfd);
            exit(0);
    }

    printf("Phase 1: Authentication request from Patient2 with username %s and password %s has been sent to the Health Center Server.\n", patient1->username, patient1->password);

    // Receive authentication response from healthcenter server  
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
    perror("recv");
    exit(1);
    }

    if(!strcmp(buf,"success"))
    {
         printf("Phase 1: Patient 2 authentication result: success.\n");
            memset(buf, 0, sizeof(buf));
            strcpy(buf,"available");
            // Send available request to healthcenterserver upon successful authntication.
            if (send(sockfd, buf, sizeof(buf), 0) == -1){
                perror("send");
                close(sockfd);
                exit(0);
           }
    }
    else{

            printf("Phase 1: Patient2 authentication result: failure.\n");
            close(sockfd);
            exit(0);
    }

    printf("Phase 1: End of Phase 1 for Patient2.\n");

    memset(buf, 0, sizeof(buf));
    if ((numbytes = recv(sockfd, buf, sizeof(buf), 0)) == -1) {
    perror("recv");
    exit(1);
    }

    if(!strcmp(buf, "notavailable")){
    printf("Phase 2: The following appointments are available for Patient 1: Not Available\n");
    close(sockfd);
    exit(0);
    }

    char options[6];
    char option;
    char buf1[MAXDATASIZE];
    memset(buf1, 0, sizeof(buf1));
    strcpy(buf1,buf);
    memset(options, 0, sizeof(options));
    getavailableoptions(buf1, options);


    start_p = buf;
    char *start_p2 = buf;
    int i=0;

    printf("Phase 2: The following appointments are available for Patient 2:\n");

    while(1){

       tab_p = strchr(start_p2, ' ');
       if(tab_p == NULL) break;
       start_p2 = tab_p+1; i++;
       if( i == 3 ){
       *tab_p++ = '\0';
       printf("%s\n", start_p);
       start_p = tab_p;
       start_p2 = tab_p;
       i=0;
    }

  }

    printf("Please enter the preferred appointment index and press enter:");


        i=0;
        int flag1 =0,flag2 =0;
        while(1){
            scanf("%c",&option);
                while(i<= strlen(options) ){
                    if(option == options[i]){
                    flag1 =1;
                    break;
                }
                i++;
            }
            if(flag1)
                break;
            if(!flag2){
                printf("Invalid Selection. Please  enter  the  preferred  appointment  index  and  press  enter:");
                flag2 =1;
            }
            i=0;
        }

                memset(buf, 0, sizeof(buf));
                strcpy(buf, "selection ");
                strncat(buf, &options[i],1);

                if (send(sockfd, buf, sizeof(buf), 0) == -1){
                perror("send");
                close(sockfd);
                exit(0);
                 }

    memset(buf, 0, sizeof(buf));
    if ((numbytes = recv(sockfd, buf, sizeof(buf), 0)) == -1) {
    perror("recv");
    exit(1);
    }

    if(!strcmp(buf, "notavailable")){
        printf("Phase 2: The requested appointment from Patient 2 is not available.  Exiting...\n");
        close(sockfd);
        exit(0);
    }

    close(sockfd);

    start_p = strchr(buf, ' ');
    *start_p++ = '\0';

    doctorinfo_t doctorinfo;
    strcpy(doctorinfo.doctor,buf);
    strcpy(doctorinfo.port,start_p);

    int port = atoi(doctorinfo.port) + 720;

    printf("Phase 2: The requested appointment is available and reserved to Patient2. The  assigned doctor port number is %d.\n", port);

    sockfd=socket(AF_INET,SOCK_DGRAM,0);

    struct hostent *h_ptr = gethostbyname("localhost");

   servaddr.sin_family = AF_INET;
   servaddr.sin_port=htons(port);
   memcpy((void *)&servaddr.sin_addr, h_ptr->h_addr, h_ptr->h_length);

   memset(buf, 0, sizeof(buf));
   strcpy(buf, patient1->insurance);

   //Send insurance plan to doctor server 
   if((numbytes=sendto(sockfd,buf,sizeof(buf),0,(struct sockaddr *)&servaddr,sizeof(servaddr))) == -1){
                perror("sendto");
                close(sockfd);
                exit(0);
                 }

                 if(getsockname(sockfd, (struct sockaddr*)&localAddress,  &addressLength)){
                        perror("getsockname");
                        exit(1);
                    }

               printf("Phase 3: Patient 2 has a dynamic UDP port number %d and IP address %s.\n",(int)ntohs(localAddress.sin_port), inet_ntoa(servaddr.sin_addr));
               printf("Phase 3: The cost estimation request from Patient 2 with insurance plan %s has been sent to the doctor with port number %d and IP address %s.\n",patient1->insurance, port, inet_ntoa(servaddr.sin_addr));

                 memset(buf, 0, sizeof(buf));

   // Receive cost estimation from doctor
                if ((numbytes =  recvfrom(sockfd,buf,MAXDATASIZE,0,NULL,NULL)) == -1) {
                perror("recvfrom");
                close(sockfd);
                exit(1);
                }

                printf("Phase 3: Patient 2 receives %s$ estimation cost from doctor with port number %d and name %s.\n", buf,port,doctorinfo.doctor);
				printf("Phase 3: End of Phase 3 for Patient 2.\n"); 
                return 0;

}
