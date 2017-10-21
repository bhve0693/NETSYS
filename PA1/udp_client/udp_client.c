/*
*
* FileName        :    udp_client.c
* Description     :    This file contains SW implementation of UDP client, implemented through socket programming with the following features:
                       Handling Get, Put, Delete, ls and Exit requests to server with a reliability mechanism implemented through Stop and Wait
                       and Retransmission enabled through FRAME_KIND_ERR_RETRAS macro. The SW also implements XOR based encryption with a 
                       private mask XOR_PRIVATE_MASK
*                                             
* File Author Name:    Bhallaji Venkatesan 
* Tools used      :    gcc, Sublime Text 
* References      :    NETSYS Class Lectures
*
*/

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
#include <sys/socket.h>
#include <resolv.h>
#include <netinet/in.h>
#include <string.h> 
#include <dirent.h>


/* #define Macros */

#define FRAME_KIND_ACK         0
#define FRAME_KIND_DATA        1
#define FRAME_KIND_REQ_GET     2
#define FRAME_KIND_REQ_PUT     3
#define FRAME_KIND_REQ_LS      4
#define FRAME_KIND_REQ_DELETE  5
#define FRAME_KIND_REQ_EXIT    6
#define FRAME_KIND_ERR_FNF     7
#define FRAME_KIND_ERR_INVR    8
#define FRAME_KIND_ERR_INVS    9
#define FRAME_KIND_RES_OK     10
#define FRAME_KIND_ERR_RETRAS 11    
#define FRAME_KIND_ERR_INVCMD 12
#define XOR_BYTE            0x9E

/*Typedef Macros */

typedef struct packet{
  char data[1024];
}Packet;

typedef struct frame{
  int frame_kind;
  int seq_no;
  int ack;
  long sizer;
  Packet packet;
}FRAME;


/* Function Implementations */

char * xorBuffer(char *buffer, long bufferSize){

    int i;
    for(i = 0;i <= bufferSize;i++){
        buffer[i] ^= XOR_BYTE;
    }
    return buffer;
}

void get_file(int sockfd, long length,struct sockaddr_in servaddr, char *buff)
{
   printf("\n Entering get\n");
  long count = 0;
  int frame_id = 0;
  long temp_size = 0;
  char *temp_buff = malloc(sizeof(char)*1024);
  FRAME frame_recv;
  FRAME frame_send;
  frame_send.seq_no = 0;
  frame_send.frame_kind = FRAME_KIND_REQ_GET;
  frame_send.ack = 0;
  frame_send.sizer = length; 
  temp_size = sizeof(frame_send) - (1024-length);
  memcpy(frame_send.packet.data,buff,length); 
  sendto(sockfd, &frame_send, sizeof(frame_send), 0,
          (struct sockaddr *)&servaddr, sizeof(struct sockaddr));
  int addr_size = sizeof(struct sockaddr);
    
  int nbrecv = 0;   
  int flag = 1; 
  char new_file[]="Remote";

  strcat(new_file,buff);
  FILE *fp;
  fp=fopen(new_file,"wb");
  long rec = 0;
  long count_2 = 0;
  long count_3 = 0;
  int timeout = 0;

  while(flag == 1)
    {
      if((rec = recvfrom(sockfd,&frame_recv,sizeof(frame_recv),0,(struct sockaddr *)&servaddr, &addr_size)) < 0)
      {
        timeout = 1;
      }

      printf("Timeout after every reception %d \n",timeout);
      printf("Frame ID after every reception %d \n",frame_id);
      count_2 += rec; 
      count_3++;
      count += frame_recv.sizer;
      if(frame_recv.frame_kind == FRAME_KIND_ERR_FNF)
      {
        printf("\nFile Not Found \n");
        flag = 0;
      }
      else if(frame_recv.frame_kind == FRAME_KIND_ERR_FNF)
      {
        printf("\nFile Read Error in Server Side!! \n");
        flag = 0;
      }
      else if(frame_recv.frame_kind == FRAME_KIND_ERR_INVS)
      {
        printf("\nFile Send Error in Server Side!! \n");
        flag = 0;
      }
      else if(rec > 0 && frame_recv.frame_kind == FRAME_KIND_DATA && frame_recv.seq_no == frame_id)
      { 
        printf("FRAME Received : %s \n",frame_recv.packet.data);
        frame_send.seq_no = 0;
        frame_send.frame_kind = FRAME_KIND_ACK;
        frame_send.ack =frame_recv.seq_no +1 ;
        sendto(sockfd,&frame_send,sizeof(frame_send),0,(struct sockaddr *)&servaddr,addr_size);
        printf("ACK Sent \n");
        printf("Packet ID %d \n", frame_id);
        temp_buff = xorBuffer(frame_recv.packet.data,frame_recv.sizer);
        memcpy(frame_recv.packet.data,temp_buff, frame_recv.sizer);
        fwrite(&frame_recv.packet.data, 1, frame_recv.sizer, fp);
        timeout = 0;
        frame_id++;
      }
      else if(frame_recv.frame_kind == FRAME_KIND_ERR_RETRAS)
      {
        frame_id = frame_recv.seq_no;
      }
      else{
        printf("Client Packet ID : %d", frame_id);
        printf("Frame Not Received \n");
        printf("Timeout in else loop of ACK is %d \n",timeout);
      } 
      if(!timeout)
      {
        
        printf("Frame ID after no timeout %d \n",frame_id);
      }
    
      if(frame_recv.sizer <1024)
      {
        flag =0;
      } 

    }
    count_2 = count_2 - (count_3*24);
    fclose(fp);
}
 
void put_file(int sockfd,long length,struct sockaddr_in servaddr, char *buff)
{
  
  FRAME frame_send;
  FRAME frame_recv;
  int ack_recv = 1;
  int frame_id = 0;
  long read_len = 0;
  long count = 0;
  FILE *fp;
  int addr_size = sizeof(struct sockaddr);
  frame_send.frame_kind = FRAME_KIND_REQ_PUT;
  frame_send.sizer = length;

  memcpy(frame_send.packet.data,buff,length); 
  sendto(sockfd, &frame_send, sizeof(frame_send), 0,
          (struct sockaddr *)&servaddr, sizeof(struct sockaddr));
  
  fp=fopen(buff,"rb");
  if(fp==NULL)
    {
      printf("file does not exist\n");
      frame_send.frame_kind = FRAME_KIND_ERR_FNF;
      sendto(sockfd, &frame_send, sizeof(frame_send), 0,
          (struct sockaddr *)&servaddr, sizeof(struct sockaddr));
      exit(1);
    }
  
  fseek(fp,0,SEEK_END);
  size_t file_size=ftell(fp);
  printf("Size is %ld \n",file_size);
  fseek(fp,0,SEEK_SET);
  for(int j=100;j>=0;j--);
  while(!feof(fp)){
      if(ack_recv == 1)
      {
          frame_send.seq_no = frame_id;
          frame_send.frame_kind = FRAME_KIND_DATA;
          frame_send.ack = 0;
          read_len = fread(frame_send.packet.data,1,1024, fp);
          if(read_len == -1)
          {
            printf("Invalid Read \n");
            frame_send.frame_kind = FRAME_KIND_ERR_INVR;
      sendto(sockfd, &frame_send, sizeof(frame_send), 0,
          (struct sockaddr *)&servaddr, sizeof(struct sockaddr));
            exit(1);
          }
          count += read_len;
          frame_send.sizer = read_len;
          long temp_size = sizeof(frame_send) -(1024 - read_len);
          long sent_bytes = sendto(sockfd, &frame_send, temp_size, 0, (struct sockaddr *)&servaddr, addr_size);
          printf("Packet ID %d",frame_id);
          printf("Sent Bytes %ld", sent_bytes);
          if( sent_bytes == -1)
          {
            fprintf(stderr, "error while sending data!\n");
            frame_send.frame_kind = FRAME_KIND_ERR_INVS;
      sendto(sockfd, &frame_send, sizeof(frame_send), 0,
          (struct sockaddr *)&servaddr, sizeof(struct sockaddr));
            exit(-1);
          }
        }
        int rec = recvfrom(sockfd,&frame_recv, sizeof(frame_recv), 0, (struct sockaddr*)&servaddr, &addr_size);
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
        for(int i =1000; i>=0; i--);
    }
    fclose(fp); 
 }

void server_ls(int sockfd,struct sockaddr_in servaddr)
{
  char *buff = malloc(sizeof(char)*2000);
  long count = 0;
  int frame_id = 0;
  long temp_size = 0;
  FRAME frame_recv;
  FRAME frame_send;
  frame_send.seq_no = 0;
  frame_send.frame_kind = FRAME_KIND_REQ_LS;
  frame_send.ack = 0;
  frame_send.sizer = 0; 
  temp_size = sizeof(frame_send) - 1024;
  printf("\nsSending ls request \n");
  sendto(sockfd, &frame_send, sizeof(frame_send), 0,
          (struct sockaddr *)&servaddr, sizeof(struct sockaddr));
  int addr_size = sizeof(struct sockaddr);
    
  int nbrecv = 0;   
  int flag = 1; 
  char new_file[]="ls.txt";
  FILE *fp;
  fp=fopen(new_file,"wb");
  long rec = 0;
  long count_2 = 0;
  long count_3 = 0;

  while(flag == 1)
    {

    
      if((rec = recvfrom(sockfd,&frame_recv,sizeof(frame_recv),0,(struct sockaddr *)&servaddr, &addr_size)) == -1)
      {
          fprintf(stderr, "fail while receiving data! \n");
          exit(-1);
      }
      count_2 += rec; 
      count_3++;
      count += frame_recv.sizer;
      if(rec > 0 && frame_recv.frame_kind == FRAME_KIND_DATA && frame_recv.seq_no == frame_id)
      { 
        frame_send.seq_no = 0;
        frame_send.frame_kind = FRAME_KIND_ACK;
        frame_send.ack =frame_recv.seq_no +1 ;
        sendto(sockfd,&frame_send,sizeof(frame_send),0,(struct sockaddr *)&servaddr,addr_size);
        fwrite(&frame_recv.packet.data, 1, frame_recv.sizer, fp);
      }
      else{
        printf("Frame Not Received \n");
      } 
      frame_id++;  

      if(frame_recv.sizer <1024)
      {
        flag =0;
      } 

    }
    count_2 = count_2 - (count_3*24);
    fclose(fp);
    fp=fopen(new_file,"rb");
    printf("\nOutput of ls in server is \n");
    while(!feof(fp))
    {
       fread(buff,1,1024, fp);
       printf("\n %s \n",buff);
    }
    fclose(fp);

}

void delete_file(int sockfd, long length,struct sockaddr_in servaddr, char *buff)
{
  long count = 0;
  int frame_id = 0;
  long temp_size = 0;
  FRAME frame_recv;
  FRAME frame_send;
  frame_send.seq_no = 0;
  frame_send.frame_kind = FRAME_KIND_REQ_DELETE;
  frame_send.ack = 0;
  frame_send.sizer = length; 
  temp_size = sizeof(frame_send) - (1024-length);
  memcpy(frame_send.packet.data,buff,length); 
  sendto(sockfd, &frame_send, sizeof(frame_send), 0,
          (struct sockaddr *)&servaddr, sizeof(struct sockaddr));

}
int main(int argc, char * argv[])
{
    char buff[2000];
    int sockfd,connfd,len;
    char *original_buff = malloc(sizeof(char)*200);
    long count = 0;
    int frame_id = 0;
    char cmd_buff[200];
    int addr_size = sizeof(struct sockaddr);
    long rec = 0;
    DIR *dir;
    struct dirent *ent;
    char *name_buff = malloc(sizeof(ent->d_name)*2000);
    char *split = malloc(sizeof(char)*200);
    char *ref_input = malloc(sizeof(char)*200);;
    FRAME frame_recv;
    FRAME frame_send;
    struct sockaddr_in servaddr,cliaddr;
    if (argc < 3)
   {
      printf("USAGE:  <server_ip> <server_port>\n");
      exit(1);
   }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd==-1)
    {
      printf(" socket not created in client\n");
      exit(0);
    }
    else
   {
      printf("socket created in  client\n");
   }
   bzero(&servaddr, sizeof(servaddr)); 
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr = inet_addr(argv[1]);  // ANY address or use specific address
   servaddr.sin_port =    htons(atoi(argv[2]));  // Port address
   
   struct timeval tv;
  tv.tv_sec = 50;
  tv.tv_usec = 0;
  if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
    perror("Error");
  }

   printf("\n Enter any of the following commands! \n");
   printf("\n1. get [file_name] \n"); 
   printf("\n2. put [file_name] \n");
   printf("\n3. delete [file_name] \n");
   printf("\n4. ls \n");
   printf("\n5. exit \n");
   fgets(cmd_buff, sizeof(cmd_buff),stdin);
   memcpy(original_buff,cmd_buff,sizeof(cmd_buff));
   split = strtok(cmd_buff, " ");
   memcpy(ref_input,split,strlen(split)-1);
   if(!(strcmp(split,"get")))
   {
       split = strtok(NULL, " ");
       memcpy(buff,split,strlen(split)-1);
       long length  = strlen(buff);
       get_file(sockfd,length,servaddr,buff);
   }
   else if(!(strcmp(ref_input,"exit")))
   {
       frame_send.frame_kind = FRAME_KIND_REQ_EXIT;   
       sendto(sockfd, &frame_send, sizeof(frame_send), 0,
          (struct sockaddr *)&servaddr, sizeof(struct sockaddr));
   }
   else if(!(strcmp(split,"put")))
   {
       split = strtok(NULL, " ");
       memcpy(buff,split,strlen(split)-1);
       long length  = strlen(buff);
       put_file(sockfd,length,servaddr,buff);
   }
   else if(!(strcmp(ref_input,"ls")))
   {
        printf("List  %s", ref_input);  
        server_ls(sockfd,servaddr);
   }
   else if(!(strcmp(split,"delete")))
   {
       split = strtok(NULL, " ");
       memcpy(buff,split,strlen(split)-1);
       long length  = strlen(buff);
       delete_file(sockfd,length,servaddr,buff);
        if((rec = recvfrom(sockfd,&frame_recv,sizeof(frame_recv),0,(struct sockaddr *)&servaddr, &addr_size)) == -1)
       {
          fprintf(stderr, "fail while receiving data! \n");
          exit(-1);
       }
       if( frame_recv.frame_kind == FRAME_KIND_ERR_FNF)
       {
          printf("\nFile Not found in server to be deleted!! \n");
       }
       if(frame_recv.frame_kind == FRAME_KIND_RES_OK)
       {
          printf("\nFile Successfully deleted in Server! \n");
       }
   }
   else
   {
       frame_send.frame_kind = FRAME_KIND_ERR_INVCMD;
       frame_send.sizer = sizeof(original_buff);
       memcpy(frame_send.packet.data,original_buff,sizeof(original_buff));
       sendto(sockfd, &frame_send, sizeof(frame_send), 0,
          (struct sockaddr *)&servaddr, sizeof(struct sockaddr));
        if((rec = recvfrom(sockfd,&frame_recv,sizeof(frame_recv),0,(struct sockaddr *)&servaddr, &addr_size)) == -1)
        {
           fprintf(stderr, "fail while receiving data! \n");
           exit(-1);
        }
        printf("\n Server Returned : %s ", original_buff);
       if( frame_recv.frame_kind == FRAME_KIND_ERR_INVCMD)
       {
          char *err_buff = malloc(sizeof(char)*1024);
          memcpy(err_buff,frame_recv.packet.data,frame_recv.sizer); 
          printf("%s\n",frame_recv.packet.data);
       }
   }

    close(sockfd);
 
return(0);
}