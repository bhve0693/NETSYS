/*
*
* FileName        :    webserver.c
* Description     :    This file contains SW implementation of a HTTP-based webserver that accepts multiple
*					   simultaneous connections                                           
* File Author Name:    Bhallaji Venkatesan 
* Tools used      :    gcc, Sublime Text, Webbrowser (Mozzila Firfox) 
* References      :    NETSYS Class Lectures 
*					   http://www.binarytides.com/server-client-example-c-sockets-linux/
*					   http://www.csc.villanova.edu/~mdamian/sockets/echoC.htm
*
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/time.h>
#include <signal.h>

#define MAX_CONN 100
#define MAX_WSBUFF 1000


char *DIR_ROOT = NULL;
char PORT_NUM[8];
int sock = 0;
int clients[MAX_CONN];


void signal_handler(int signal)
{
	switch (signal)
	{
		case SIGINT:
				close(sock);
				exit(1);
				break;
		case SIGTERM:
				close(sock);
				exit(1);
				break;

	}
}


static uint32_t get_sizeof_file (FILE * fileDesc)
{
    uint32_t size;

    fseek(fileDesc, 0L, SEEK_END);
    size = ftell(fileDesc);
    fseek(fileDesc, 0L, SEEK_SET);

    return size;
}

void wsconf_read()
{
	FILE *fp = NULL;
	char wsbuff[MAX_WSBUFF];
	char *wsconfig = getenv("PWD");
	char *temp_buff = NULL;
	strncat(wsconfig,"/ws.conf", 8);
	fp=fopen(wsconfig,"r");
	if(!fp)
	{
		printf("\n Unable to find ws.conf file! Exiting the program \n");
		exit(1);
	}
	else
	{
		uint32_t wsconfig_size = get_sizeof_file (fp);
        printf("ws.conf size = %d, filename = %s\n", wsconfig_size, wsconfig);
        while(fgets(wsbuff,wsconfig_size,fp)!=NULL) {

        	/* Parsing ws.conf for finding PORT NUM */
            if(strncmp(wsbuff,"Listen",6)==0) 
            {
                temp_buff=strtok(wsbuff," \t\n");
                temp_buff = strtok(NULL, " \t\n");
                strcpy(PORT_NUM, temp_buff);
                printf("PORT NUM %s\n", PORT_NUM);
            	bzero(wsbuff, sizeof(wsbuff));
            }

            /* Parsing ws.conf for finding Root directory */
            if(strncmp(wsbuff,"DocumentRoot",12)==0)
            {
                temp_buff=strtok(wsbuff," \t\n");
                temp_buff = strtok(NULL, " \t\n");
                DIR_ROOT=(char*)malloc(100);
                strcpy(DIR_ROOT,temp_buff);
                printf("ROOT DIRECTORY : %s\n",DIR_ROOT);
                bzero(wsbuff, sizeof(wsbuff));

            }

        }

        fclose(fp);
	}
}
void start_server(int port)
{
	struct sockaddr_in sin;
	bzero((char *)&sin, sizeof(sin));
	sin.sin_family = AF_INET;							//set family as the internet family (AF_INET)
	sin.sin_addr.s_addr = INADDR_ANY;					//set any address as the address for the socket
	sin.sin_port = htons(port);							//set the port number to the socket
	if((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)	//open socket  and check if error occurs
	{
		printf("Error in creating socket \n");
		exit(1);
	}
	int optval = 1;
  	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));		//prevents the "already in use" issue
	printf("Socket created\n");
	if ((bind(sock, (struct sockaddr *)&sin, sizeof(sin)))	 < 0)		//bind socket and check if error occurs
	{
		printf("Bind Error : %d\n", bind(sock, (struct sockaddr *)&sin, sizeof(sin)));
		exit(1);
	}
	printf("Socket bound\n");	
	if(listen(sock, 4) != 0)							//listen for incoming requests on the socket we have created
	{
		printf("Error in listen\n");
		exit(1);
	}
	printf("Listening for connections\n");	
}


void conn_response(int conn_count)
{
	printf("\nEntering Conn Response \n");
}

int main(int argc, char *argv[])
{
	int conn_count = 0;
	struct sockaddr_in clientAddr;
    socklen_t addrlen;
    struct sigaction custom_signal;

    custom_signal.sa_handler = signal_handler;

    if(sigfillset(&custom_signal.sa_mask) == -1)
    {
    	printf("\nERR:Signal Mask Setting Failed!\n");
		return 1;
	}

	if(sigaction(SIGINT, &custom_signal, NULL) == -1)
    {
    	printf("\nERR:Cannot Handle SIGINT!\n");
    	return 1;
    }
	if(sigaction(SIGTERM, &custom_signal, NULL) == -1)
    {
    	printf("\nERR:Cannot Handle SIGTERM!\n");
    	return 1;
	}
	wsconf_read();

    for(int i= 0; i<MAX_CONN; i++)
    {
    	clients[i] = -1; //Setting all socket values as -1 
    }
    int port = atoi(PORT_NUM);
    if(port < 1024)
    {
    	printf("\n Chosen Port is invalid (Occupied by system defaults) \n");
    	exit(1);
    }
    start_server(port);

    while(1)
    {
    	addrlen = sizeof(clientAddr);
        clients[conn_count] = accept (sock, (struct sockaddr *) &clientAddr, &addrlen);
        printf("%d\n",clients[conn_count]);
        if(clients[conn_count] < 0)
        {
        	printf("\n Error in Connection Accept! \n");
        }
        else
        {
        	if(!fork())
        	{
        		conn_response(conn_count);
        	}

        	while (clients[conn_count]!=-1) 
            {
                conn_count = (conn_count+1)%MAX_CONN;
            }
        }

    }
}