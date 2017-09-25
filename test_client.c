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
  //fp=fopen(buff,"rb");
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
          //printf("Enter Packet: ");
          //scanf("%s", buffer);
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
       // int addr_size = sizeof(remote);
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
        //if(frame_id > 1024)
        //  break;
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
  printf("Sending ls request \n");
  sendto(sockfd, &frame_send, sizeof(frame_send), 0,
          (struct sockaddr *)&servaddr, sizeof(struct sockaddr));
 // sendto(sockfd, buff, strlen(buff), 0,
  //        (struct sockaddr *)&servaddr, sizeof(struct sockaddr));
      //char file_buffer[1000];
  int addr_size = sizeof(struct sockaddr);
    //long read_len = recvfrom(sockfd,file_buffer,140531,0,(struct sockaddr *)&servaddr, &addr_size);
    
  int nbrecv = 0;   
  int flag = 1; 
  char new_file[]="ls.txt";
   //char file_full_buffer[150000];
  FILE *fp;
  fp=fopen(new_file,"wb");
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
      printf("Count2_ full frame %ld",count_2);
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
    fclose(fp);
    fp=fopen(new_file,"rb");
    printf("\nOutput of LS in server is \n");
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

    // create socket in client side
    
 /*  FILE *fp;
   fp=fopen(ls_file,"wb");
    if ((dir = opendir ("./")) != NULL) {
    while ((ent = readdir (dir)) != NULL) {
      printf ("%s\n", ent->d_name);

      fwrite(ent->d_name, 1, strlen(ent->d_name), fp);
      char temp[] = "\n";
      fwrite(temp,1,sizeof(temp)-1,fp);

      memcpy(name_buff+(k*sizeof(ent->d_name)), ent->d_name, sizeof(ent->d_name));
      k++;
    }
    closedir (dir);
    } else {
  perror ("");
  exit(1);
}
fclose(fp);
fp=fopen(ls_file,"rb");
while(!feof(fp))
{
  fread(buff,1,1024, fp);
  printf("Output of LS is %s \n",buff);
}
fclose(fp);

int j = k;*/
/*while(k)
{  
printf("Listing %s\n",name_buff+((j-k)*sizeof(ent->d_name)));
k--;
}
printf("Size of Overall Struct %ld\n",j*sizeof(ent->d_name));*/

//exit(1);

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
   servaddr.sin_addr.s_addr = inet_addr(argv[1]);  //inet_addr("128.138.201.66"); // ANY address or use specific address
   servaddr.sin_port =    htons(atoi(argv[2]));  // Port address
   printf("\n Enter any of the following commands! \n");
   printf("\n1. get [file_name] \n"); 
   printf("\n2. put [file_name] \n");
   printf("\n3. delete [file_name] \n");
   printf("\n4. ls \n");
   printf("\n5. exit \n");
   //scanf("%s",cmd_buff);
   fgets(cmd_buff, sizeof(cmd_buff),stdin);
   printf("\n%s",cmd_buff);
   split = strtok(cmd_buff, " ");
   printf("Split String %s", split);
   memcpy(ref_input,split,strlen(split)-1);
   if(!(strcmp(split,"get")))
   {
       split = strtok(NULL, " ");
       printf("Split String after delimiter %s", split);
       memcpy(buff,split,strlen(split)-1);
       printf("\n length %ld \n",strlen(buff));
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
       printf("Split String after delimiter %s", split);
       memcpy(buff,split,strlen(split)-1);
       printf("\n length %ld \n",strlen(buff));
       long length  = strlen(buff);
       put_file(sockfd,length,servaddr,buff);
      // get_file(sockfd,length,servaddr,buff);
   }
   else if(!(strcmp(ref_input,"ls")))
   {
        printf("List  %s", ref_input);  
        server_ls(sockfd,servaddr);
   }
   else if(!(strcmp(split,"delete")))
   {
       split = strtok(NULL, " ");
       printf("Split String after delimiter %s", split);
       memcpy(buff,split,strlen(split)-1);
       printf("\n length %ld \n",strlen(buff));
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
      // get_file(sockfd,length,servaddr,buff);
   }
   else
   {
        printf("\nInvalid Command! Exiting UDP Tranfers!! \n"); 
   }

       // printf("UDP File name message\n");
    //printf("\n length %ld \n",strlen(buff));
    //scanf("%s",buff);
  //  exit(1);
// send  msg to server
  // free(split);
/*
sendto(sockfd, buff, strlen(buff), 0,
          (struct sockaddr *)&servaddr, sizeof(struct sockaddr));
      //char file_buffer[1000];
    int addr_size = sizeof(struct sockaddr);
    //long read_len = recvfrom(sockfd,file_buffer,140531,0,(struct sockaddr *)&servaddr, &addr_size);
    
  int nbrecv = 0;   
  int flag = 1; 
   char new_file[]="Remote";
   //char file_full_buffer[150000];
  strcat(new_file,buff);
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
//   if(fp !=NULL)
//{
  // fclose(fp);
//}

  //close client side connection
    close(sockfd);
 
return(0);
}