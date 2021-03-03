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
#define DEBUG
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

  struct sockaddr_storage clientAddr;
  socklen_t addrLenght;

  char buf[256];
  char helloMsg[] = "Hello 1.0\n";
  char operation[20];
  char nickName[100];
  int nbytes;
  char remoteIP[INET6_ADDRSTRLEN];


  struct nameAndId
  {
    char nick[13];
    int id;
  };

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

  for(p = servinfo; p != NULL; p->ai_next)
  {
    listenSocket = socket(p->ai_family,p->ai_socktype,p->ai_protocol);
    if(listenSocket < 0)
    {
      continue;
    }
    if(bind(listenSocket,p->ai_addr,p->ai_addrlen) < 0)
    {
      close(listenSocket);
      continue;
    }
    break;
  }
  setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int));

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
    if(select(maxFds +1,&read_fds,NULL,NULL,NULL) == -1)
    {
      printf("Select error\n");
      continue;
    }

    
    for(i = 0; i <= maxFds; i++)
    {
      if(FD_ISSET(i,&read_fds))
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
            acceptFd > maxFds;
          }
          #ifdef DEBUG
          printf("new Connection\n");
          #endif
          if(sendValue = send(i,helloMsg,sizeof(helloMsg),0) < 0)
          {
            printf("Error sending hello msg\n");
            continue;
          }
          #ifdef DEBUG
          printf("Hello msg sent\n");
          #endif
          }

        }
        else
        {
          //handle data from client

          if(recivedValue = recv(i,buf,sizeof(buf),0) < 0)
          {
            printf("recive error");
            continue;
          }
          //read operation + nickname from recived buf
          sscanf(buf,"%s %s",operation, nickName);
          #ifdef DEBUG
          printf("operation = %s\n", operation);
          #endif
          if(strcmp(operation,"NICK") == 0)
          {
            //check NICK and sendback error or ok
            printf("Inside Nick check. Nick = %s\n",nickName);
            char *expression="^[A-Za-z_]+$";
            regex_t regularexpression;
            int reti;
            
            reti=regcomp(&regularexpression, expression,REG_EXTENDED);
            if(reti)
            {
              fprintf(stderr, "Could not compile regex.\n");
              exit(1);
            }
            
            int matches;
            regmatch_t items;

        
  
            for(j=2;j<strlen(nickName);j++)
            {
              if(strlen(nickName)<12)
              {
                reti=regexec(&regularexpression, nickName,matches,&items,0);
                if(!reti)
                {
                  //nick accepted send back ok
                  printf("Nick %s is accepted.\n",nickName);
                  memset(buf,0,sizeof(buf));
                  sprintf(buf,"OK %s\n",nickName);
                  if(sendValue = send(i,buf,strlen(buf),0) < 0)
                  {
                    printf("Error sending ok for nickname\n");
                  }
                  #ifdef DEBUG
                  printf("sent ok msg size: %d\n",sendValue);
                  #endif
                } 
                else 
                {
                  //nick rejected send back ERR <txt>
                  printf("%s is not accepted.\n",nickName);
                  memset(buf,0,sizeof(buf));
                  sprintf(buf,"ERR %s\n",nickName);
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
                strcpy(buf,"ERR to long nickname\n");
                if(sendValue = send(i,buf,strlen(buf),0) < 0)
                {
                  printf("Error sending ERR for to long nickname\n");
                }
                #ifdef DEBUG
                printf("sent to long nick: %d\n",sendValue);
                #endif
              }
            }            
            regfree(&regularexpression);
            free(org);
            memset(buf,0,sizeof(buf)); 
          }
          else if(strcmp(operation,"MSG") == 0)
          {
            //Echo sent msg to all connected servers!
            for(k = 0; k <= maxFds; k++)
            {
              
            }
          }
          else
          {
            //rubbish recived do somthing? 
          }
        

        }
      }
    }
  }
  
  
  return(0);
}