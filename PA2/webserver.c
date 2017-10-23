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

/***** MACROS *******/

#define MAX_CONN         100
#define MAX_WSBUFF 	     1000
#define TX_BYTES		 1024
#define CLIENT_MSGSIZE 	 65536
#define TRUE             1
#define FALSE            0
#define STATUSLENGTH     2000


/**** Global Variables ****/

char *DIR_ROOT = NULL;
char PORT_NUM[8];
int sock = 0;
int clients[MAX_CONN];
int COUNT = 0;
int client_count = 0;
int internal_error = 0;

/*******Function Implementations***********/

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


char * content_check(char *filetype)
{
    char *type;

    if((strcmp(filetype,".htm"))==0 || (strcmp(filetype,".html"))==0)
        type = "text/html";
    else if((strcmp(filetype,".jpg"))==0)
        type = "image/jpeg";
    else if(strcmp(filetype,".gif")==0)
        type = "image/gif";
    else if(strcmp(filetype,".txt")==0)
        type = "text/plain";
    else if(strcmp(filetype,".png")==0)
        type = "image/png";
    else if(strcmp(filetype,".css")==0)
        type = "text/css";
    else if(strcmp(filetype,".js")==0)
        type = "text/javascript";
    else 
        type="application/octet-stream";

    return type;

}

int format_extract(char *filetype)
{

    FILE *fp;
    char wsbuff[MAX_WSBUFF];
    unsigned int formatIndex = 0;
    int file_supported = 0;
    char formats[20][100];
 

    char *wsconfig = getenv("PWD");
	if (wsconfig != NULL)
    	printf("Path to ws.conf: %s \n", wsconfig);

    fp=fopen(wsconfig,"r");
    unsigned int wsconfig_size = get_sizeof_file (fp);

    while(fgets(wsbuff,wsconfig_size,fp)!=NULL) {//read from the .conf file
        strcpy(formats[formatIndex],wsbuff);
        formatIndex++;
    }

    int k=0;

    
    for(k=0;k<formatIndex+1;k++) {
        if(strncmp(formats[k],filetype,3)==0) {//check if the file is supported
            file_supported = 1;//if supported then set file_supported.
            break;
        }
    }

    fclose(fp);

    return file_supported;


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
                if(!DIR_ROOT)
                {
                	internal_error = 1;  // Internal Error for HTTP 500 Error Handling
                	break;
                }
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


void conn_response(int n)
{
	printf("\nEntering Conn Response \n");
	char msg_from_client[CLIENT_MSGSIZE], *req_string[3], msg_to_client[TX_BYTES], path[CLIENT_MSGSIZE];
    int rcvd, fd, bytes_read;
    FILE *fp;
    char status[STATUSLENGTH];
    char connection_status[50];
    client_count = n;
    int flag_post=0;

	printf("\n _________________________________\nclient_count : %d\n",client_count);
	while(1){

	flag_post = 0;  // make it zero before every post request
	bzero(status, sizeof(status));
    bzero(msg_from_client, sizeof(msg_from_client));
    bzero(path, sizeof(path));
    bzero(req_string, sizeof(req_string));
    bzero(msg_to_client, sizeof(msg_to_client));
    bzero(connection_status, sizeof(connection_status));

    memset( (void*)msg_from_client, (int)'\0', CLIENT_MSGSIZE );
    rcvd=recv(clients[n], msg_from_client, CLIENT_MSGSIZE, 0);


	char filename[50] = "Msgstore";
    char count_str[50];
    sprintf(count_str,"%d", COUNT);
    strcat(filename, count_str);
    FILE *fp_storeMsg = fopen(filename, "w");
    if (fp_storeMsg != NULL)
    {
        fputs(msg_from_client, fp_storeMsg);
        fclose(fp_storeMsg);
    }
	if (!strstr(msg_from_client,"Connection: Keep-alive"))    // capturing the last string from the received message
	{
	    strncpy(connection_status, "Connection: Keep-alive", strlen("Connection: Keep-alive"));
	}
	else    /* -- If Keep-alive is not found, close the connection --- */
	{
		strncpy(connection_status, "Connection: Close",strlen("Connection: Close"));
	}

    bzero(status, sizeof(status));
    if (rcvd<0)    // receive error
        fprintf(stderr,("recv() error\n"));
    else if (rcvd==0)    // receive socket closed
    	rcvd = 0;


    else    // message received
    {

    	if (!strstr(msg_from_client,"Connection: Keep-alive"))    // capturing the last string from the received message
		{
	        strncpy(connection_status, "Connection: Keep-alive", strlen("Connection: Keep-alive"));
		}
		else    /* -- If Keep-alive is not found, close the connection --- */
		{
			strncpy(connection_status, "Connection: Close",strlen("Connection: Close"));
		}

        printf("\n printing msg_from_client %s\n", msg_from_client);
        printf("Size of rcvd message %ld \n", sizeof(msg_from_client));

        // Now breaking the incoming strng into three different paths
        req_string[0] = strtok (msg_from_client, " \t\n");
        if ((strncmp(req_string[0], "GET\0", 4)==0) || (strncmp(req_string[0], "POST\0", 5)==0))
        {
        	if (strncmp(req_string[0], "POST\0", 5)==0)
        	{
        		printf("____________________________________________\n");
        		flag_post = TRUE;
        	}

            req_string[1] = strtok (NULL, " \t");
            req_string[2] = strtok (NULL, " \t\n");
            

            char http_version[8];
            if (strncmp(req_string[2], "HTTP/1.1", 8) == 0)
                strcpy(http_version, "HTTP/1.1");
            else
                strcpy(http_version, "HTTP/1.0");
            if(internal_error)
            {
            	strncat(status,http_version,strlen(http_version));
                strncat(status," 500 Internal Server Error:Cannot allocate memory",strlen(" 500 Internal Server Error"));
                strncat(status,"\n",strlen("\n"));
                strncat(status,"<html><body>>500 Internal Server Error:Cannot allocate memory",strlen("<body>>400 Bad Request Reason: Invalid HTTP-Version :"));
                strncat(status,"HTTP",strlen("HTTP"));
                strncat(status,"</body></html>",strlen("</body></html>"));
                strncat(status,"\r\n",strlen("\r\n"));
                printf("%s\n",status);
               	write(clients[n], status, strlen(status));
               	internal_error = 0;

            }

            if ( strncmp( req_string[2], "HTTP/1.0", 8)!=0 && strncmp( req_string[2], "HTTP/1.1", 8)!=0 )
            {


                strncat(status,http_version,strlen(http_version));
                strncat(status," 400 Bad Request",strlen(" 400 Bad Request"));
                strncat(status,"\n",strlen("\n"));
                strncat(status,"<html><body>>400 Bad Request Reason: Invalid HTTP-Version :",strlen("<body>>400 Bad Request Reason: Invalid HTTP-Version :"));
                strncat(status,"HTTP",strlen("HTTP"));
                strncat(status,"</body></html>",strlen("</body></html>"));
                strncat(status,"\r\n",strlen("\r\n"));
                printf("%s\n",status);
               	write(clients[n], status, strlen(status));
            }
            else
            {
                if ( strncmp(req_string[1], "/\0", 2)==0 )
                    req_string[1] = "/index.html";        //Because if no file is specified, index.html will be opened by default (like it happens in APACHE...

                strcpy(path, DIR_ROOT);
                strcpy(&path[strlen(DIR_ROOT)], req_string[1]);
                printf("file: %s\n", path);

                int formatCheck;
                char *ext = strrchr (path, '.');
                if (ext == NULL)
                {
                    formatCheck = FALSE;
                }
                else
                {
                    formatCheck = format_extract(ext);
                }
                
                
                char size_array[20];

                
                if (formatCheck == TRUE)
                {
                    if ( (fd=open(path, O_RDONLY))!=-1 )    //FILE FOUND
                    {
                        fp = fopen(path,"r");;
                        char *checkfileType = content_check(ext);
                        int size= get_sizeof_file(fp);
                        sprintf(size_array,"%d",size);
                        
                        char msg_post[CLIENT_MSGSIZE];
                        if (flag_post)
                        {
                        	COUNT++;
                        	printf("coming into post loop\n");
                        	FILE * fp_msgcheck;
						    char * line = NULL;
						    size_t len = 0;
						    ssize_t read;
						    int line_empty = FALSE;
						    fp_msgcheck = fopen(filename, "r");
						    if (fp_msgcheck == NULL)
						        exit(EXIT_FAILURE);

						    int temp_var = 1;

						    while ((read = getline(&line, &len, fp_msgcheck)) != -1) {
						    
						    	if (read == 2 && line_empty == FALSE)
						    	{
						    		line_empty = TRUE;

						    	}

						   		if (line_empty == TRUE)
						    	{
						    		printf("			Retrieved line of length %zu :\n", read);
						        	printf("			%s", line);
						        	if (temp_var == 1)
						        	{
						        		
						        		temp_var = 0;
						        	}
						        	else
						        	{
						        		
						    			strncat(msg_post, line, strlen(line));
						    			printf("Coming here!!%s\n", msg_post);
						    		}
						    		
						    	}

						    }
						    printf("						%s\n", msg_post);	
						    temp_var = 1;
						    line_empty = FALSE;
						    fclose(fp_msgcheck);
						    remove(filename);
						    if (line)
						        free(line);
                        }

                        char msg[CLIENT_MSGSIZE];
                        if (flag_post)
                        {
                        	strncat(status,"POST ",strlen("POST "));	
                        }
                        strncat(status,http_version,strlen(http_version));
                        strncat(status," 200 OK",strlen(" 200 OK"));
                        strncat(status,"\n",strlen("\n"));
                        strncat(status,"Content-Type:",strlen("Content-type:"));
                        strncat(status,checkfileType,strlen(checkfileType));
                        strncat(status,"\n",strlen("\n"));
                        strncat(status,"Content-Length:",strlen("Content-Length:"));
                        strncat(status,size_array,strlen(size_array));
                        strncat(status,"\n",strlen("\n"));
                        strncat(status,connection_status,strlen(connection_status));
                        if (flag_post)
                        {
	                        strncat(status,"\r\n\r\n",strlen("\r\n\r\n"));
	                        sprintf(msg,"<h1>POST DATA</h1><html><body>%s</body></html>\n",msg_post);
	                        strncat(status, msg, strlen(msg));
	                        strncat(status,"\r\n",strlen("\r\n"));
	                    }
	                    else{
	                        strncat(status,"\r\n",strlen("\r\n"));
	                        strncat(status,"\r\n",strlen("\r\n"));
	                    }
                        printf("printing status update to client %s\n",status);
                        send(clients[n], status, strlen(status), 0);

                        while ( (bytes_read=read(fd, msg_to_client, TX_BYTES))>0 )
                            write (clients[n], msg_to_client, bytes_read);
                        printf("\n Loaded Index.html \n");

                        fclose(fp);
                        bzero(msg_post,sizeof(msg_post));
                        bzero(msg,sizeof(msg));
                    }
                
                    else{  // file not found loop

                    	strncat(status,http_version,strlen(http_version));
                        strncat(status," 404 Not Found",strlen(" 404 Not Found"));
                        strncat(status,"\n",strlen("\n"));
                        strncat(status,"Content-Type:",strlen("Content-type:"));
                        strncat(status,"Invalid",strlen("Invalid"));
                        strncat(status,"\n",strlen("\n"));
                        strncat(status,"Content-Length:",strlen("Content-Length:"));
                        strncat(status,"Invalid",strlen("Invalid"));
	                	strncat(status,"\n",strlen("\n"));
		            	strncat(status,connection_status,strlen(connection_status));                     
                        strncat(status,"\r\n",strlen("\r\n"));
                        strncat(status,"\r\n",strlen("\r\n"));
                        strncat(status,"<html><body>404 Not Found: URL does not exist:",strlen("<body>404 Not Found: URL does not exist:"));
                        strncat(status,path,strlen(path));
                        strncat(status,"</body></html>",strlen("</body></html>"));
                        strncat(status,"\r\n",strlen("\r\n"));
                        printf("%s\n",status);
                        write(clients[n], status, strlen(status)); //FILE NOT FOUND
                            
                    }    
                        
                }

                else // file not supported
                {
                	printf("***************************************************\n");
                	strncat(status,http_version,strlen(http_version));
                    strncat(status," 501 Not Implemented",strlen(" 501 Not Implemented"));
                    strncat(status,"\n",strlen("\n"));//strncat(status_line,"\r\n",strlen("\r\n"));
                    strncat(status,"Content-Type:",strlen("Content-type:"));
                    strncat(status,"NONE",strlen("NONE"));
                    strncat(status,"\n",strlen("\n"));
                    strncat(status,"Content-Length:",strlen("Content-Length:"));
                    strncat(status,"NONE",strlen("NONE"));
                	strncat(status,"\n",strlen("\n"));
	            	strncat(status,connection_status,strlen(connection_status));                    
                    strncat(status,"\r\n",strlen("\r\n"));
                    strncat(status,"\r\n",strlen("\r\n"));
                    strncat(status,"<HEAD><TITLE>501 Not Implemented</TITLE></HEAD>",strlen("<HEAD><TITLE>501 Not Implemented</TITLE></HEAD>"));
               
                    strncat(status,"<body>501 Not Implemented: File format not supported:",strlen("<body>501 Not Implemented: File format not supported:"));
                    strncat(status,http_version,strlen(http_version));
                    strncat(status,"</body></html>",strlen("</body></html>"));
                   
                    strncat(status,"\r\n",strlen("\r\n"));
                    write(clients[n], status, strlen(status)); //FILE NOT FOUND   
                }

            }
            

        }

        else
            {//file not found

                strncat(status,"HTTP/1.1",strlen("HTTP/1.1"));
                strncat(status,"\n",strlen("\n"));
                strncat(status,"Content-Type:",strlen("Content-type:"));
                strncat(status,"NONE",strlen("NONE"));
                strncat(status,"\n",strlen("\n"));
                strncat(status,"Content-Length:",strlen("Content-Length:"));
                strncat(status,"NONE",strlen("NONE"));
                strncat(status,"\r\n",strlen("\r\n"));
                strncat(status,"\r\n",strlen("\r\n"));
                strncat(status,"<HEAD><TITLE>501 Not Implemented</TITLE></HEAD>",strlen("<HEAD><TITLE>501 Not Implemented</TITLE></HEAD>"));
                strncat(status,"<body>501 Not Implemented: File format not supported:",strlen("<body>501 Not Implemented: File format not supported:"));
                strncat(status,"HTTP/1.1",strlen("HTTP/1.1"));
                strncat(status,"</body></html>",strlen("</body></html>"));
                strncat(status,"\r\n",strlen("\r\n"));
                write(clients[n], status, strlen(status));   
             }
    }

	if (!strstr(msg_from_client,"Connection: Keep-alive"))    
	{

	}
	else  
	{
	    shutdown (clients[n], SHUT_RDWR);         // Send and recv disabled
    	close(clients[n]);
    	clients[n]=-1;
    	exit(0);
	}
}

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

    while(1) // Server Running forever 
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