#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>
#include <file.h>
#define FRAME_KIND_DATA 1
#define FRAME_KIND_ACK 0
#define MAXBUFSIZE 1024

/* You will have to modify the program below */
typedef struct packet{
	char data[1024];
}Packet;

typedef struct frame{
	int frame_kind;
	int seq_no;
	int ack;
	Packet packet;
}FRAME;

int main (int argc, char * argv[])
{

	int nbytes;                             // number of bytes send by sendto()
	int sock;                               //this will be our socket
	char buffer[MAXBUFSIZE];
    int length;
	struct sockaddr_in remote;              //"Internet socket address structure"
	FRAME frame_send;
	FRAME frame_recv;
	int ack_recv = 1;
	int frame_id = 0;
	if (argc < 3)
	{
		printf("USAGE:  <server_ip> <server_port>\n");
		exit(1);
	}

	/******************
	  Here we populate a sockaddr_in struct with
	  information regarding where we'd like to send our packet 
	  i.e the Server.
	 ******************/
	bzero(&remote,sizeof(remote));               //zero the struct
	remote.sin_family = AF_INET;                 //address family
	remote.sin_port = htons(atoi(argv[2]));      //sets port to network byte order
	remote.sin_addr.s_addr = inet_addr(argv[1]); //sets remote IP address
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    printf("getting picture size \n");
    FILE *picture;
    picture = fopen(argv[3],"r");
    int temp_size;
    fseek(picture, 0, SEEK_END);
    temp_size = ftell(picture);
    fseek(picture, 0, SEEK_SET);

    printf("Sending picture size");
    //Frame Header should contain Size, extension etc..,

	//Causes the system to create a generic socket of type UDP (datagram)
	//if ((sock = **** CALL SOCKET() HERE TO CREATE A UDP SOCKET ****) < 0)
	if(sock < 0)
	{
		printf("unable to create socket");
	}
    while(1){
    	if(ack_recv == 1)
    	{
    	    frame_send.seq_no = frame_id;
    	    frame_send.frame_kind = FRAME_KIND_DATA;
    	    frame_send.ack = 0;
    	    printf("Enter Packet: ");
    	    scanf("%s", buffer);
    	    strcpy(frame_send.packet.data,buffer);
    	    sendto(sock, &frame_send, sizeof(frame_send),0, (struct sockaddr*)&remote, sizeof(remote));
        }
        int addr_size = sizeof(remote);
        int rec = recvfrom(sock,&frame_recv, sizeof(frame_recv), 0, (struct sockaddr*)&remote, &addr_size);
        if(rec > 0 && frame_recv.frame_kind == FRAME_KIND_ACK && frame_recv.seq_no == 0 && frame_recv.ack == frame_id+1)
        {
        	printf("ACK Received\n");
        	ack_recv = 1;
        }
        else
        {
        	printf("ACK NOT Received\n");
        	ack_recv = 0;

        }
        frame_id++;
        //if(frame_id > 1024)
        //	break;
    }
    close(sock);



	/******************
	  sendto() sends immediately.  
	  it will report an error if the message fails to leave the computer
	  however, with UDP, there is no error if the message is lost in the network once it leaves the computer.
	 ******************/
	// char command[] = "apple";
	// length = sizeof(struct sockaddr);	
	// nbytes = sendto(sock, command, strlen(command),0,&remote, length);
	//  if(nbytes < 0)
	//  {
	//  	printf("Error in sending \n");
	//  }
	// //nbytes = **** CALL SENDTO() HERE ****;

	// // Blocks till bytes are received
	// struct sockaddr_in from_addr;
	// int addr_length = sizeof(struct sockaddr);
	// bzero(buffer,sizeof(buffer));
	// nbytes = recvfrom(sock,buffer,100,0,&remote, &addr_length); 
	// //nbytes = **** CALL RECVFROM() HERE ****;  

	// printf("Server says %s\n", buffer);

	// close(sock);

}

