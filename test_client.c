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

#define FRAME_KIND_DATA 1
#define FRAME_KIND_ACK 0
 
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

 

 
int main()
{
    char buff[2000];
    int sockfd,connfd,len;
    long count = 0;
    int frame_id = 0;
  FRAME frame_recv;
  FRAME frame_send;
struct sockaddr_in servaddr,cliaddr;
 
// create socket in client side
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
servaddr.sin_addr.s_addr = INADDR_ANY; // ANY address or use specific address
servaddr.sin_port = htons(7864);  // Port address
 

    printf("UDP File name message\n");
    printf("\n length %ld \n",strlen(buff));
    scanf("%s",buff);
    printf("\n length %ld \n",strlen(buff));
// send  msg to server
 printf("\n length %ld \n",strlen(buff));
sendto(sockfd, buff, strlen(buff), 0,
          (struct sockaddr *)&servaddr, sizeof(struct sockaddr));
      char file_buffer[1000];
    int addr_size = sizeof(struct sockaddr);
    //long read_len = recvfrom(sockfd,file_buffer,140531,0,(struct sockaddr *)&servaddr, &addr_size);
    
  int nbrecv = 0;   
  int flag = 1; 
   char new_file[]="Vihanga";
   char file_full_buffer[150000];
  strcat(new_file,buff);
  FILE *fp_client;
  char test_file[] = "test_client.jpg";
  fp_client = fopen(test_file,"r");
  FILE *fp;
  fp=fopen(new_file,"w");
long rec = 0;
long count_2 = 0;
long count_3 = 0;
//long count = 0;
while(flag == 1)
  {
    //memset(file_buffer, '\0', 1024);
    
    if((rec = recvfrom(sockfd,&frame_recv,sizeof(frame_recv),0,(struct sockaddr *)&servaddr, &addr_size)) == -1)
    {
        fprintf(stderr, "fail while receiving data! \n");
        exit(-1);
    }
    count_2 += rec; 
    count_3++;
    printf("Count2_ full fram %ld",count_2);
    //count_2 = count_2 -frame_recv.sizer;
    //printf("Count2_ size other than frame %ld",count_2);
    count += frame_recv.sizer;
    if(rec > 0 && frame_recv.frame_kind == FRAME_KIND_DATA && frame_recv.seq_no == frame_id)
    { 
      printf("FRAME Received : %s \n",frame_recv.packet.data);
        frame_send.seq_no = 0;
        frame_send.frame_kind = FRAME_KIND_ACK;
        frame_send.ack =frame_recv.seq_no +1 ;
        sendto(sockfd,&frame_send,sizeof(frame_send),0,(struct sockaddr *)&servaddr,addr_size);
        printf("ACK Sent \n");
        printf("Packet ID %d \n", frame_id);
        fwrite(&frame_recv.packet.data, 1, frame_recv.sizer, fp);
        //fwrite(&frame_recv.packet.data, frame_recv.sizer,1, fp);
        //strcat(file_buffer,frame_recv.packet.data);
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
  printf("Size of Count_2 %ld",count_2);
printf("Size of receive file %ld bytes",count);
//fwrite(file_buffer, 1, count, fp);

/*
while(flag == 1)
{

    memset(file_buffer, '\0', 1024);

    if((nbrecv = recvfrom(sockfd, file_buffer, 1024, 0, (struct sockaddr *)&servaddr, &addr_size)) == -1)
    {
        fprintf(stderr, "fail while receiving data! \n");
        exit(-1);
    }

    count += nbrecv;
    printf("n = %d\n", nbrecv);
    strcat(file_full_buffer,file_buffer);
    //fwrite(file_buffer, 1, nbrecv, fp);

    if(nbrecv<1024)
    {
        flag = 0;
    }

} 
   fwrite(file_full_buffer, 1, count, fp);


/*
    if (read_len<0)
    {
      printf("error in recieving the file\n");
      exit(1);
    }
 
  char new_file[]="recvd";
  strcat(new_file,buff);
  FILE *fp;
  fp=fopen(new_file,"w+");
  //if(fwrite(file_buffer,1,sizeof(file_buffer),fp)<0)
    if(fwrite(file_buffer,1,read_len,fp)<0)
    {
      printf("error writting file\n");
      exit(1);
    } */
if(fp !=NULL)
{
   fclose(fp);
}

  //close client side connection
    close(sockfd);
 
return(0);
}