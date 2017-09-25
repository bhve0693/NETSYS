#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>

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
#define XOR_BYTE            0x9E

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

char * xorBuffer(char *buffer, long bufferSize){

    int i;
    for(i = 0;i <= bufferSize;i++){
        buffer[i] ^= XOR_BYTE;
    }
    return buffer;
}


void client_get_file(int sd,struct sockaddr_in cliaddr,int len,FRAME frame_send, FRAME frame_recv)
{
  char buff[200];
  //FRAME frame_send;
  //FRAME frame_recv;
  int ack_recv = 1;
  int frame_id = 0;
  long read_len = 0;
  long count = 0;
  char *temp_buff = malloc(sizeof(char)*1024);
  printf("%s\n",buff);
  printf("Frame recvd : %s\n  ", frame_recv.packet.data);
  memcpy(buff,frame_recv.packet.data, frame_recv.sizer);
  printf("%s\n",buff);
  /* */
  FILE *fp;
  fp=fopen(buff,"rb");
  //fp=fopen(buff,"rb");
  if(fp==NULL)
    {
      printf("file does not exist\n");
      frame_send.frame_kind = FRAME_KIND_ERR_FNF;
      sendto(sd, &frame_send, sizeof(FRAME), 0, (struct sockaddr *)&cliaddr, len);
      close(sd);
      exit(1);
    }
  
  fseek(fp,0,SEEK_END);
  size_t file_size=ftell(fp);
  printf("Size is %ld \n",file_size);
  fseek(fp,0,SEEK_SET);

  while(!feof(fp)){
      if(ack_recv == 1)
      {
          
          //printf("Enter Packet: ");
          //scanf("%s", buffer);
          read_len = fread(frame_send.packet.data,1,1024, fp);
          if(read_len == -1)
          {
            printf("Invalid Read \n");
            frame_send.frame_kind = FRAME_KIND_ERR_INVR;
            sendto(sd, &frame_send, sizeof(FRAME), 0, (struct sockaddr *)&cliaddr, len);
            close(sd);
            exit(1);
          }
          frame_send.seq_no = frame_id;
          frame_send.frame_kind = FRAME_KIND_DATA;
          frame_send.ack = 0;
          count += read_len;
          printf("Read Length is %ld",read_len);
          frame_send.sizer = read_len;
          printf("n = %ld\n", read_len);
          //printf("File Buffer %s", file_buffer);
          //strncpy(frame_send.packet.data,file_buffer,read_len);
          printf("FFrame Buffer %s", frame_send.packet.data);
          temp_buff = xorBuffer(frame_send.packet.data,read_len);
          memcpy(frame_send.packet.data ,temp_buff,read_len);
          //strcpy(frame_send.packet.data,file_buffer);
          //long packet_len = strlen(frame_send.packet.data);
          //printf("Packet_Len sent is %ld \n",packet_len);
          long temp_size = sizeof(frame_send) -(1024 - read_len);
          long sent_bytes = sendto(sd, &frame_send, temp_size, 0, (struct sockaddr *)&cliaddr, len);
          printf("Packet ID %d",frame_id);
          printf("Sent Bytes %ld", sent_bytes);
          if( sent_bytes == -1)
          {
            fprintf(stderr, "error while sending data!\n");
            frame_send.frame_kind = FRAME_KIND_ERR_INVS;
            sendto(sd, &frame_send, sizeof(FRAME), 0, (struct sockaddr *)&cliaddr, len);
            close(sd);

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
    fclose(fp); 
 }

void client_put_file(int sd, long length,struct sockaddr_in cliaddr, FRAME frame_send, FRAME frame_recv)
{
  long count = 0;
  int frame_id = 0;
  char buff[200];
  long temp_size = 0;
 
  printf("Frame recvd : %s\n  ", frame_recv.packet.data);
  memcpy(buff,frame_recv.packet.data, frame_recv.sizer);
 // sendto(sockfd, buff, strlen(buff), 0,
  //        (struct sockaddr *)&servaddr, sizeof(struct sockaddr));
      //char file_buffer[1000];
  int addr_size = sizeof(struct sockaddr);
    //long read_len = recvfrom(sockfd,file_buffer,140531,0,(struct sockaddr *)&servaddr, &addr_size);
    
  int nbrecv = 0;   
  int flag = 1; 
  char new_file[]="Remote";
   //char file_full_buffer[150000];
  strcat(new_file,buff);
  FILE *fp;
  fp=fopen(new_file,"wb");
  long rec = 0;
  long count_2 = 0;
  long count_3 = 0;
//long count = 0;

  while(flag == 1)
    {
      //memset(file_buffer, '\0', 1024);
    
      if((rec = recvfrom(sd,&frame_recv,sizeof(frame_recv),0,(struct sockaddr *)&cliaddr, &addr_size)) == -1)
      {
          fprintf(stderr, "fail while receiving data! \n");
          exit(-1);
      }
      count_2 += rec; 
      count_3++;
      printf("Received FRAME kind is %d \n",frame_recv.frame_kind);
      printf("Count2_ full fram %ld",count_2);
      //count_2 = count_2 -frame_recv.sizer;
      //printf("Count2_ size other than frame %ld",count_2);
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
        sendto(sd,&frame_send,sizeof(frame_send),0,(struct sockaddr *)&cliaddr,addr_size);
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
    fclose(fp);


}

void client_ls_req(int sd,struct sockaddr_in cliaddr)
{
   DIR *dir;
   struct dirent *ent;
   int k =0;
   FILE *fp;
   FRAME frame_send;
   FRAME frame_recv;
   int ack_recv = 1;
   int frame_id = 0;
   long read_len = 0;
   long count = 0;
   char ls_file[] = "ls.txt";
   int len = sizeof(struct sockaddr);
   fp=fopen(ls_file,"wb");
    if ((dir = opendir ("./")) != NULL) {
     /* print all the files and directories within directory */
    while ((ent = readdir (dir)) != NULL) {
    printf ("%s\n", ent->d_name);
    fwrite(ent->d_name, 1, strlen(ent->d_name), fp);
    char temp[] = "\n";
    fwrite(temp,1,sizeof(temp)-1,fp);
    }
    closedir (dir);
    } 
    else {
      /* could not open directory */
      perror ("");
      exit(1);
    }
    fclose(fp);

  fp=fopen(ls_file,"rb");
  //fp=fopen(buff,"rb");
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
          read_len = fread(frame_send.packet.data,1,1024, fp);
          if(read_len == -1)
          {
            printf("Invalid Read \n");
            exit(1);
          }
          count += read_len;
          printf("Read Length is %ld",read_len);
          frame_send.sizer = read_len;
          printf("n = %ld\n", read_len);
          //printf("File Buffer %s", file_buffer);
          //strncpy(frame_send.packet.data,file_buffer,read_len);
          printf("FFrame Buffer %s", frame_send.packet.data);
          //strcpy(frame_send.packet.data,file_buffer);
          //long packet_len = strlen(frame_send.packet.data);
          //printf("Packet_Len sent is %ld \n",packet_len);
          long temp_size = sizeof(frame_send) -(1024 - read_len);
          long sent_bytes = sendto(sd, &frame_send, temp_size, 0, (struct sockaddr *)&cliaddr, len);
          printf("Packet ID %d",frame_id);
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
    fclose(fp); 
}

int main(int argc, char * argv[])
{
  char buff[200];
  //char file_buffer[140531];
  int sd,connfd,len;
  long count,read_len =0;
  struct sockaddr_in servaddr,cliaddr;
  FRAME frame_send;
  FRAME frame_recv;
  int ack_recv = 1;
  int frame_id = 0;
  if (argc != 2)
  {
    printf ("USAGE:  <port>\n");
    exit(1);
  }
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
  servaddr.sin_port = htons(atoi(argv[1]));
 
  if ( bind(sd, (struct sockaddr *)&servaddr, sizeof(servaddr)) != 0 )
    printf("Not binded\n");
  else
    printf("Binded\n");
 

 
  len=sizeof(cliaddr);
  recvfrom(sd,&frame_recv,sizeof(frame_recv),0,(struct sockaddr *)&cliaddr, &len);
  //recvfrom(sd,buff,1024,0,s
  // (struct sockaddr *)&cliaddr, &len);
  printf("File %s to be deleted",frame_recv.packet.data);
  printf("Request kind is %d\n",frame_recv.frame_kind);
  if(frame_recv.frame_kind == FRAME_KIND_REQ_GET)
  {
    client_get_file(sd,cliaddr,len,frame_send,frame_recv);
  }
  if(frame_recv.frame_kind == FRAME_KIND_REQ_EXIT)
  {
    printf("\nReceived exit request from Client! \n");
    printf("\nExiting from Server!! \n");
    close(sd);
    exit(1);
  }  
  if(frame_recv.frame_kind == FRAME_KIND_REQ_PUT)
  {
    client_put_file(sd, len,cliaddr, frame_send, frame_recv);
    //client_put_file(sd,cliaddr,len,frame_send,frame_recv);
  }
  if(frame_recv.frame_kind == FRAME_KIND_REQ_LS)
  {
    client_ls_req(sd,cliaddr);
  }
  if(frame_recv.frame_kind == FRAME_KIND_REQ_DELETE)
  {
    memcpy(buff,frame_recv.packet.data, frame_recv.sizer);
    printf("\n buff is %s \n",buff);
    if(!(remove(buff)))
    {
      printf("File %s is deleted",frame_recv.packet.data);
      frame_send.frame_kind = FRAME_KIND_RES_OK;
      sendto(sd, &frame_send, sizeof(FRAME), 0, (struct sockaddr *)&cliaddr, len);
    }
    else
    {
      printf("File not found \n");
      frame_send.frame_kind = FRAME_KIND_ERR_FNF;
      sendto(sd, &frame_send, sizeof(FRAME), 0, (struct sockaddr *)&cliaddr, len);
    }

  }
/*  printf("%s\n",buff);
  printf("Frame recvd : %s\n  ", frame_recv.packet.data);
  memcpy(buff,frame_recv.packet.data, frame_recv.sizer);
  printf("%s\n",buff);
  
  FILE *fp;
  fp=fopen(buff,"rb");
  //fp=fopen(buff,"rb");
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
          read_len = fread(frame_send.packet.data,1,1024, fp);
          if(read_len == -1)
          {
            printf("Invalid Read \n");
            exit(1);
          }
          count += read_len;
          printf("Read Length is %ld",read_len);
          frame_send.sizer = read_len;
          printf("n = %ld\n", read_len);
          //printf("File Buffer %s", file_buffer);
          //strncpy(frame_send.packet.data,file_buffer,read_len);
          printf("FFrame Buffer %s", frame_send.packet.data);
          //strcpy(frame_send.packet.data,file_buffer);
          //long packet_len = strlen(frame_send.packet.data);
          //printf("Packet_Len sent is %ld \n",packet_len);
          long temp_size = sizeof(frame_send) -(1024 - read_len);
          long sent_bytes = sendto(sd, &frame_send, temp_size, 0, (struct sockaddr *)&cliaddr, len);
          printf("Packet ID %d",frame_id);
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
  //printf("%ld bytes sent. \n", count);

  //printf("File sent!!\n"); 

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
 // bzero(file_buffer,sizeof(file_buffer));
 // if(fp != NULL)
  //{
    //fclose(fp);  
  //}
 // }
  
  /* */
  close(sd);
 
  return(0);
}