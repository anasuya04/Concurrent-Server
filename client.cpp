// Client side C/C++ program to demonstrate Socket programming 
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <netdb.h> 
using namespace std;
void udpclient(char* add,char* udpport);//for udp clients
int main(int argc, char *argv[]) 
{ 
	int sock,port,n;
	struct sockaddr_in serv_addr;
    socklen_t sockaddr_len = sizeof(struct sockaddr);
    struct hostent *server; 
	char *hello = "Hello from client"; 
	char buffer[1024] = {0}; 
	if (argc < 3) {
       fprintf(stderr,"usage %s hostname port\n", argv[0]);
       exit(0);
    }
    port = atoi(argv[2]);
    //creating tcp soket
	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
	{ 
		printf("\n Socket creation error \n"); 
		return -1; 
	} 
	//take hostname as argument and return the pointer to a hostant which contains information about that host
	server = gethostbyname(argv[1]);
	if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    //initializing the server information
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET; 
	bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
	serv_addr.sin_port = htons(port); 
	//connect() system call used to establish the connetion with server
	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
	{ 
		printf("\nConnection Failed \n"); 
		return -1; 
	}
	char r[]="getudp";
	char a[7];
	while(1)
	{
	printf("Please enter the message for server: ");
    bzero(buffer,1024);
    fgets(buffer,1023,stdin);
    //send the request msg to the server
	n=write(sock , buffer , strlen(buffer)); 
	if(n<0)
		printf("Error in writing soket");
	else 
	{
		for(int i=0;i<7;i++)
		a[i]=buffer[i];
	    a[6]='\0';
	    //check the request msg is getudp or not
		if(strcmp(a,r) == 0)
		{
			bzero(buffer,1024);
			//resd the udp port from server and close tcp connection
	        n=read(sock,buffer,1023);
	        if(n<0)
		    printf("Error in reading from soket");
	        else printf("Udpport rcvd from server using tcp  :%s\n",buffer);
            printf("---------------close tcp------------------\n");
            close(sock);
            udpclient(argv[1],buffer);//call udpclient
            
            break;
        }
        printf("Type 1 msg send to server using tcp :%s\n",buffer);
     }
	bzero(buffer,1024);
	//read response from server
	n=read(sock,buffer,1023);
	if(n<0)
		printf("Error in reading from soket");
	else printf("type 2 msg rcvd from server using tcp:%s\n",buffer);
	
	}
	return 0; 
}
//udpclient
void udpclient(char* add,char* udpport)
{
	int sock_fd,port,n;
	struct hostent *server; 
    struct sockaddr_in serv_addr;
    socklen_t sockaddr_len = sizeof(struct sockaddr);
    port = atoi(udpport);
    char buffer[1024]={0};
    //create soket for udp
    if ((sock_fd = socket (AF_INET, SOCK_DGRAM, 0)) <0 ) {
        perror("Problem in creating the socket");
        exit(EXIT_FAILURE);
    }
    //take hostname as argument and return the pointer to a hostant which contains information about that host
    server = gethostbyname(add);
	if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }
    //fill server family
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET; 
	bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
	serv_addr.sin_port = htons(port);
	char r[]="quitudp";
	char a[8];
	int len;
	while(1)
	{
	printf("Please enter the message for udp server: ");
    bzero(buffer,1024);
    fgets(buffer,1023,stdin);
    //send msg from udpclient
	n=sendto(sock_fd, buffer, strlen(buffer), 
        MSG_CONFIRM, (const struct sockaddr *) &serv_addr,  
            sizeof(serv_addr)); 
	if(n<0)
		printf("Error in writing soket");
	else 
	{
		for(int i=0;i<7;i++)
		a[i]=buffer[i];
	    a[7]='\0';
	    //check send msg is quitudp or not
		if(strcmp(a,r) == 0)
		{
			bzero(buffer,1024);
			//recv last msg from server and close udp
	        n=recvfrom(sock_fd, buffer, 1024,  
                MSG_WAITALL, (struct sockaddr *) &serv_addr, 
                (socklen_t*)&len);
	        if(n<0)
		    printf("Error in reading from soket");
	        else printf("final msg rcvd from server :%s\n",buffer);
            printf("--------------------close udp---------------------\n");
            close(sock_fd);//close udp soket
            break;
        }
        printf("Type 3 msg send to server using udp :%s\n",buffer);
     }
	bzero(buffer,1024);
	//recv response from udp server
	n=recvfrom(sock_fd, buffer, 1024,  
                MSG_WAITALL, (struct sockaddr *) &serv_addr, 
                (socklen_t*)&len);
	if(n<0)
		printf("Error in reading from soket");
	else printf("type 4 msg rcvd from server using udp :%s\n",buffer);
	
	}

}

