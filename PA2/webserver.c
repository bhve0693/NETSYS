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

#define MAX_CONN 100
#define MAX_WSBUFF 1000


char *DIR_ROOT = NULL;
char PORT_NUM[8];
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

int main(int argc, char *argv[])
{
	wsconf_read();
}