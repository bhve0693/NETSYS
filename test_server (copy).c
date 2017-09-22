#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>


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
  char buff[8];
  char file_buffer[140531];
  int sd,connfd,len;
  long count,read_len =0;
  struct sockaddr_in servaddr,cliaddr;
  FRAME frame_send;
  FRAME frame_recv;
  int ack_recv = 1;
  int frame_id = 0;

  sd = socket(AF_INET, SOCK_DGRAM, 0);
 
  if(sd==-1)
    {
      printf(" socket not created in server\n");
      exit(0);
    }
  else
    {
      printf("socket created in  server\n");
    }
 
  bzero(&servaddr, sizeof(servaddr));
 
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = INADDR_ANY;
  servaddr.sin_port = htons(7834);
 
  if ( bind(sd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0 )
    printf("Not binded\n");
  else
    printf("Binded\n");
 

 
  len=sizeof(cliaddr);
 
  recvfrom(sd,buff,1024,0,
   (struct sockaddr *)&cliaddr, &len);
  
  printf("%s\n",buff);

  /* */
  FILE *fp;
  fp=fopen(buff,"rb");
  if(fp==NULL)
    {
      printf("file does not exist\n");
      exit(1);
    }
  
  fseek(fp,0,SEEK_END);
  size_t file_size=ftell(fp);
  printf("Size is %ld \n",file_size);
  fseek(fp,0,SEEK_SET);

while(!feof(fp)){
      if(ack_recv == 1)
      {
          frame_send.seq_no = frame_id;
          frame_send.frame_kind = FRAME_KIND_DATA;
          frame_send.ack = 0;
          //printf("Enter Packet: ");
          //scanf("%s", buffer);
          read_len = fread(file_buffer, 1, 1024, fp);
          count += read_len;
          printf("Read Length is %ld",read_len);
          frame_send.sizer = read_len;
          printf("n = %ld\n", read_len);
          strcpy(frame_send.packet.data,file_buffer);
          long temp_size = sizeof(frame_send) -(1024 - read_len);
          long sent_bytes = sendto(sd, &frame_send, temp_size, 0, (struct sockaddr *)&cliaddr, len);

          printf("Sent Bytes %ld", sent_bytes);
          if( sent_bytes == -1)
          {
            fprintf(stderr, "error while sending data!\n");
            exit(-1);
          }
        }
       // int addr_size = sizeof(remote);
        int rec = recvfrom(sd,&frame_recv, sizeof(frame_recv), 0, (struct sockaddr*)&cliaddr, &len);
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
        //if(frame_id > 1024)
        //  break;
    }

  /* while(!feof(fp)){
        read_len = fread(file_buffer, 1, 1024, fp);
        count += read_len;
        printf("n = %ld\n", read_len);

        if(sendto(sd, file_buffer, read_len, 0, (struct sockaddr *)&cliaddr, len) == -1){
            fprintf(stderr, "error while sending data!\n");
            exit(-1);
        }
    }*/
  printf("%ld bytes sent. \n", count);

  printf("File sent!!\n"); 

  /*read_len = fread(file_buffer,1,file_size,fp);
  printf("Read Lenght %ld \n",read_len);
  if(read_len<=0)
    {
      printf("unable to copy file into buffer\n");
      exit(1);
    }
  //if(sendto(sd,file_buffer,strlen(file_buffer),0,  (struct sockaddr *)&cliaddr, len)<0) 
  if(sendto(sd,file_buffer,read_len,0,  (struct sockaddr *)&cliaddr, len)<0)
  {
    printf("error in sending the file\n");
    exit(1);
  }*/
  bzero(file_buffer,sizeof(file_buffer));
  fclose(fp);
  
  /* */
  close(sd);
 
  return(0);
}