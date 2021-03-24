#include <stdio.h>
#include <stdlib.h>
/* You will to add includes here */
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <regex.h>
#include <arpa/inet.h>

struct nameTracker
{
  char nameArr[100];
  int index;
};

struct nameTracker names[100];
int main(int argc, char *argv[]){
  
  /* Do more magic */

  if(argc!=2)
  {
    printf("To few arguments!\n");
    exit(1);
  }
  
  char *hoststring,*portstring, *rest, *org;
  org=strdup(argv[1]);
  rest=argv[1];
  hoststring=strtok_r(rest,":",&rest);
  portstring=strtok_r(rest,":",&rest);
  printf("Listening on:%s \n",org);

  struct addrinfo hints, *p, *servinfo;

  fd_set master;
  fd_set read_fds;
  int maxFds;
  int i, j, k;
  int listenSocket;
  int acceptFd;
  int nrOfNames = 0;
  struct sockaddr_storage clientAddr;
  socklen_t addrLenght;

  char buf[256];
  char sendMSG[1000];
  char helloMsg[30];
  char wrongFormatMsg[] = "Wrong format. Send MSG or NICK first in msg\n";
  char operation[20];
  char nickName[100];
  
  char tempReadableIp[INET6_ADDRSTRLEN];
  int tempPort;
  
  strcpy(helloMsg,"HELLO 1\n");

  

  int counter;

  int yes = 1;
  int recivedValue;
  int sendValue;
  FD_ZERO(&master);
  FD_ZERO(&read_fds);

  memset(&hints,0,sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;
  hints.ai_family = AF_UNSPEC;
  
  
  if(recivedValue = getaddrinfo(hoststring,portstring,&hints,&servinfo) != 0)
  {
    printf("getaddrInfo error\n");
    exit(1);
  }
  
  free(org);

  for(p = servinfo; p != NULL; p = p->ai_next)
  {
    listenSocket = socket(p->ai_family,p->ai_socktype,p->ai_protocol);
    if(listenSocket < 0)
    {
      continue;
    }
    setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int));
    if(bind(listenSocket,p->ai_addr,p->ai_addrlen) < 0)
    {
      close(listenSocket);
      continue;
    }
    break;
  }
  

  if(p == NULL)
  {
    printf("failed to bind\n");
    exit(1);
  }

  freeaddrinfo(servinfo);

  if(listen(listenSocket,20) < 0)
  {
    printf("listen error\n");
    exit(3);
  }


  FD_SET(listenSocket,&master);

  maxFds = listenSocket;
  #ifdef DEBUG
  printf("Before loop\n");
  #endif

  while (1)
  {
    read_fds = master;
    
    if(select(maxFds+1,&read_fds,NULL,NULL,NULL) == -1)
    {
      printf("Select error\n");
      exit(2);
    }

    
    for(i = 0; i <= maxFds; i++)
    {
      if(FD_ISSET(i,&read_fds))// Got connecion
      {
        if(i == listenSocket)
        {
          addrLenght = sizeof(struct sockaddr_storage);
          acceptFd =  accept(listenSocket,(struct sockaddr *)&clientAddr,&addrLenght);
          if(acceptFd == -1)
          {
            printf("accept Error\n");
          }
          else
          {
            FD_SET(acceptFd,&master);
            if(acceptFd>maxFds)
            {
              maxFds = acceptFd;
            }

            #ifdef DEBUG
            printf("new Connection\n");
            #endif
            if(sendValue = send(acceptFd,helloMsg,strlen(helloMsg),0) < 0)
            {
              printf("Error sending hello msg\n");
              //close(acceptFd);
              break;
            }
            #ifdef DEBUG
            printf("Hello msg sent\n");
            #endif
          }
          continue;
        }
        else
        {
          //handle data from client
          memset(buf,0,sizeof(buf));
          if(recivedValue = recv(i,buf,sizeof(buf),0) <= 0)
          {
            if(recivedValue == 0)
            {
              printf("recived 0\n");                           
            }
            else if(recivedValue == -1)
            {
              printf("recv error\n");              
            }
            close(i);
            FD_CLR(i,&master);     
          }
          //read operation + nickname from recived buf
          sscanf(buf,"%s %s",operation, nickName);
          #ifdef DEBUG
          //printf("operation = %s\n", operation);
          #endif
          if(strcmp(operation,"NICK") == 0)
          {
            //check NICK and sendback error or ok
            
            #ifdef DEBUG            
            printf("Inside Nick check. Nick = %s\n",nameArr[i]);
            #endif
            char *expression="^[A-Za-z_]+$";
            
            regex_t regularexpression;
            int reti;
            
            
            reti=regcomp(&regularexpression, expression,REG_EXTENDED);
            
            if(reti)
            {
              fprintf(stderr, "Could not compile regex.\n");
              exit(1);
            }
            
            int matches = 0;
            regmatch_t items;
  
            
              if(strlen(nickName)<12)
              {                
                reti=regexec(&regularexpression, nickName,matches,&items,0);
                
                if(!reti)
                {
                  //nick accepted send back ok
                  memset(names[nrOfNames].nameArr, 0, sizeof(names[nrOfNames].nameArr));
                  names[nrOfNames].index = i;
                  strcpy(names[nrOfNames++].nameArr,nickName);

                  printf("Nick %s is accepted.\n",nickName);
                  memset(buf,0,sizeof(buf));
                  strcpy(buf,"OK\n");

                  if(sendValue = send(i,buf,strlen(buf),0) < 0)
                  {
                    printf("Error sending ok for nickname\n");
                  }
                  #ifdef DEBUG
                  //printf("sent ok msg size: %d\n",sendValue);
                  #endif

                } 
                else 
                {
                  //nick rejected send back ERR <txt>
                  printf("%s is not accepted.\n",nickName);
                  memset(buf,0,sizeof(buf));
                  sprintf(buf,"ERROR %s\n",nickName);
                  if(sendValue = send(i,buf,strlen(buf),0) < 0)
                  {
                    printf("Error sending ERR msg for nickname\n");
                  }
                  #ifdef DEBUG
                  printf("sent ERRmsg bad chars size: %d\n",sendValue);
                  #endif
                }
              } 
              else 
              {
                //nickname to long send back ERR <txt>
                printf("%s is too long (%ld vs 12 chars).\n", nickName, strlen(nickName));
                memset(buf,0,sizeof(buf));
                strcpy(buf,"ERROR to long nickname\n");
                if(sendValue = send(i,buf,strlen(buf),0) < 0)
                {
                  printf("Error sending ERR for to long nickname\n");
                }
                FD_CLR(i,&master);
                close(i);

                #ifdef DEBUG
                printf("sent to long nick: %d\n",sendValue);
                #endif
              }
            regfree(&regularexpression);
         
            memset(buf,0,sizeof(buf)); 
          }
          else if(strcmp(operation,"MSG") == 0)
          {
            //Echo sent msg to all connected servers!
            char *temp = strchr(buf,' ');
            char tempName[20];
            
            for(int l = 0; l < nrOfNames; l++)
            {
              if(names[l].index == i)
              {
                strcpy(tempName,names[l].nameArr);
              }
            }
            sprintf(sendMSG,"MSG %s%s",tempName,temp);
            for(k = 0; k <= maxFds; k++)
            {
              if(FD_ISSET(k,&master))
              {
                if(k!= listenSocket && k != i)
                {
                  if(sendValue = send(k,sendMSG,strlen(sendMSG),0) < 0)
                  {
                    printf("sendError to all\n");
                    continue;
                  }                  
                }
              }           
            }
            memset(sendMSG,0,sizeof(sendMSG));
          }
          else
          {
            if(sendValue = send(i,"ERROR\n",strlen("ERROR\n"),0) < 0)
            {
              printf("sendError to all\n");
              continue;
            }   
          }
          memset(operation,0,strlen(operation));
          memset(buf,0,sizeof(buf));
        }
      }
    }
  }
  close(listenSocket);
  return(0);
}