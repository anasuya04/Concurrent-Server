// Server side C/C++ program to demonstrate Socket programming 
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h>
#include <iostream>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#define MAXCLIENTS 30
using namespace std;
//global variables will be shared between clients
static int* ports;//ports used by udp server
static int* freeports;//to keep information about which ports are free
static int* activeclient;//maintain no of clients which are concurrently served by server
int x=(rand() %(30000 - 20000 + 1)) + 20000;//randomly used port for udp
           
int udpserver(char* udpport,int clientno);//for phase2 connection
void child(int new_socket,int clientno);// to handle multiple client
//main function
int main(int argc, char *argv[]) 
{   
	ports=(int *)mmap(NULL,  MAXCLIENTS* sizeof(int), PROT_READ | PROT_WRITE,
                                MAP_SHARED | MAP_ANONYMOUS, -1, 0);//mmap used to allocate memory so that same memory space used by different client
	freeports=(int *)mmap(NULL,  MAXCLIENTS* sizeof(int), PROT_READ | PROT_WRITE,
                                MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	activeclient=(int *)mmap(NULL,sizeof(int), PROT_READ | PROT_WRITE,
                                MAP_SHARED | MAP_ANONYMOUS, -1, 0);
	for(int i=0;i<MAXCLIENTS;i++)//initializing ports array and freeports array
    {
	   ports[i]=x;
	   freeports[i]=0;
	   x++;
    }
	int server_fd,newserver_fd,new_socket,port,pid;
	struct sockaddr_in serv_addr, cli_addr; 
	int opt = 1; 
	int n;
	int addrlen = sizeof(serv_addr); 
	char buffer[1024] = {0}; 
	fd_set soketsets; 
	if (argc < 2) 
	{
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
    server_fd = socket(AF_INET, SOCK_STREAM, 0);// socket() system call creates a new socket
	if (server_fd <0) 
	{ 
		perror("socket failed"); 
		exit(EXIT_FAILURE); 
	} 
	//setsockopt used for reuse of address and port
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
												&opt, sizeof(opt))) 
	{ 
		perror("setsockopt"); 
		exit(EXIT_FAILURE); 
	} 
	memset(&serv_addr, 0, sizeof(serv_addr));
	port = atoi(argv[1]);
	serv_addr.sin_family = AF_INET; //AF_INET used for IPV4 
	serv_addr.sin_addr.s_addr = INADDR_ANY; 
	serv_addr.sin_port = htons(port); 
	//The bind() system call binds a socket to an address
	if (bind(server_fd, (struct sockaddr *)&serv_addr, 
								sizeof(serv_addr))<0) 
	{ 
		perror("bind failed"); 
		exit(EXIT_FAILURE); 
	}
	//The listen system call used to listen on the socket for connections
	if (listen(server_fd, 5) < 0) 
	{ 
		perror("listen"); 
		exit(EXIT_FAILURE); 
	} 
	printf("-----------Server is waiting for client-------------\n");
	while(1)
	{
		//The accept() system call used to block the process until a client connects to the server
		if ((new_socket = accept(server_fd, (struct sockaddr *)&cli_addr,(socklen_t*)&addrlen))<0) 
		{ 
		perror("accept"); 
		exit(EXIT_FAILURE);
		}
		//fork used to handle multiple client at the same time on server
		pid = fork();
         if (pid < 0){
             perror("ERROR on fork");
             exit(EXIT_FAILURE); 
         }
         if (pid == 0)  {
             close(server_fd);
             *activeclient+=1;
             child(new_socket,*activeclient);//if fork creation is succesful then star the communication
             exit(0);
         }
         else close(new_socket);

	}
	return 0;
}
void child(int new_socket,int clientno)
{
   int n;
   char buffer[1024];
   char *hello = "I got your message"; 
   char r[]="getudp";
   char a[7]; 
   while(1)
	{
	bzero(buffer,1024);
	//read request from client
	n = read( new_socket , buffer, 1023); 
	if (n < 0)
		printf("ERROR reading from socket");
	else printf("Type 1 msg rcvd from client %d using tcp :%s\n",clientno, buffer );
	  
	for(int i=0;i<6;i++)
		a[i]=buffer[i];
	a[6]='\0';
	//compaire received msg is getport or not
	if(strcmp(a, r) == 0){
			
            int udpport;
            //select port which is free
            for(int i = 0; i < MAXCLIENTS; i++)
            {
                if(freeports[i] == 0)
                {
                    freeports[i] = 1;
                    udpport =ports[i];
                    break;
                }
            }
            sprintf(buffer, "%d",udpport);
            printf("Udpport send to client %d using tcp :%s\n",clientno,buffer );
            //send the free port to client and close tcp connection
			n=write(new_socket , buffer , strlen(buffer));
			if (n < 0)
			printf("ERROR writing from socket");
			printf("---------------close tcp for client %d---------------\n",clientno);
			close(new_socket);//close tcp soket
			int p= udpserver(buffer,clientno);// call udp server to handle the connection
			//after udp connection close free the port
			for(int i = 0; i < MAXCLIENTS; i++)
            {
                if(ports[i] == p)
                {
                    freeports[i] = 0;
                    break;
                }
            }
            break;
        }
    printf("Type 2 response send to client %d using tcp :%s\n",clientno,buffer);
    //send the response msg to client
	n=write(new_socket , buffer , strlen(buffer));
	if (n < 0)
		printf("ERROR writing from socket");
	
	}
}
int udpserver(char* udpport,int clientno)
{
	int sockfd; 
    char buffer[1024]; 
    int port=atoi(udpport);
    char *hello = "Hello from udp server"; 
    struct sockaddr_in servaddr, cliaddr; 
      
    // Creating udp socket file descriptor 
    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
      
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 
      
    // Fill the server information 
    servaddr.sin_family    = AF_INET; // IPv4 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(port); 
      
    // bind() system call binds the socket to the server address 
    if ( bind(sockfd, (const struct sockaddr *)&servaddr,  
            sizeof(servaddr)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
      
    int len, n; 
  
    len = sizeof(cliaddr);
    char r[]="quitudp";
	char a[8];
	while(1)
	{
	bzero(buffer,1024);
	//recv the msg from client
	n=recvfrom(sockfd, buffer, sizeof(buffer), 
            0, (struct sockaddr*)&cliaddr,(socklen_t*)&len); 
	if (n < 0)
		printf("ERROR reading from socket");
	else printf("Type 3 msg rcvd from client %d using udp :%s\n",clientno, buffer);
	  
	for(int i=0;i<7;i++)
		a[i]=buffer[i];
	a[7]='\0';
	//compaire recv msg is quitudp or not
	if(strcmp(a, r) == 0){
		    //send bye to client and close udp connection
		    n=sendto(sockfd, "Bye",3, 0, 
          (struct sockaddr*)&cliaddr, sizeof(cliaddr));
		    if (n < 0)
			printf("ERROR writing from socket");
            printf("---------------close udp for client %d---------------\n",clientno);
            close(sockfd);//close udp soket
            break;
        }
    printf("Type 4 response send to client %d using udp :%s\n",clientno,buffer);
    //send response msg to client
	n=sendto(sockfd, buffer,sizeof(buffer) , 0, 
          (struct sockaddr*)&cliaddr, sizeof(cliaddr));
	if (n < 0)
		printf("ERROR writing from socket");
	
	}
	return port;
}
