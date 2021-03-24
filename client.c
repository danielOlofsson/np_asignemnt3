#include <stdio.h>
#include <stdlib.h>
/* You will to add includes here */
#include <string.h>
#include <regex.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <unistd.h>
#include <sys/time.h>


#define STDIN 0
#define DEBUG
int main(int argc, char *argv[]){
  /* GET name and port from arguments. */
  
   if(argc!=3)
  {
    printf("To few arguments!\n");
    exit(1);
  }

  char *hoststring,*portstring, *rest, *org, *nickName;
  org=strdup(argv[1]);
  rest=argv[1];
  hoststring=strtok_r(rest,":",&rest);
  portstring=strtok_r(rest,":",&rest);
  


  /* This is to test nicknames */
  char *expression="^[A-Za-z_]+$";
  regex_t regularexpression;
  int reti;
  
  reti=regcomp(&regularexpression, expression,REG_EXTENDED);
  if(reti){
    fprintf(stderr, "Could not compile regex.\n");
    exit(1);
  }
  
  int matches;
  regmatch_t items;
  
  
  for(int i=2;i<argc;i++)
  {
    if(strlen(argv[i])<12)
    {
      reti=regexec(&regularexpression, argv[i],matches,&items,0);
      if(!reti)
      {
       
        printf("Nick %s is accepted.\n",argv[i]);
       
      }
      else 
      {
        
        printf("%s is not accepted.\n Exiting\n",argv[i]);
        exit(1);
        
      }
    } 
    else 
    {
      
      printf("%s is too long (%ld vs 12 chars).\nExiting\n", argv[i], strlen(argv[i]));
      exit(1);
    }
  }
  //printf("Leaving\n");
  regfree(&regularexpression);
  free(org);
  nickName = argv[2];
  
  struct addrinfo hints, *serverinfo, *servaddr;
  fd_set readfds;
  int recivedValue;
  int clientSocket;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC; // AF_INET , AF_INET6
  hints.ai_socktype = SOCK_STREAM; // <<--- TRANSPORT PROTOCOL!!
  hints.ai_flags = AI_PASSIVE; 

  if ((recivedValue = getaddrinfo(hoststring, portstring, &hints, &serverinfo)) != 0) 
  {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(recivedValue));
		exit(2);
	}

  for(servaddr = serverinfo; servaddr != NULL; servaddr = servaddr->ai_next) 
  {
	  if ((clientSocket = socket(servaddr->ai_family, servaddr->ai_socktype, servaddr->ai_protocol)) == -1)
    {
			perror("talker: socket");
		  continue;
		}
	  break;
	}

  if (servaddr == NULL) 
  {
		fprintf(stderr, "talker: failed to create socket\n");
		exit(3);
	}

  if (connect(clientSocket,servaddr->ai_addr, servaddr->ai_addrlen) < 0 ) 
  {
	  perror("connecton error .\n");
	  exit(1);
	}
  
  char buf[256];
  char inputMsg[4100];
  char printMsg[400];
  memset(buf,0,sizeof(buf));

  // ta emot hello
  if((recivedValue = recv(clientSocket,buf,sizeof(buf),0)) == -1)
  {
    perror("sendto:");
    exit(1);
  }

  if(strcmp(buf,"HELLO 1\n") != 0)
  {
    printf("Does not support this version!\n");
    close(clientSocket);
  }

  memset(buf,0,sizeof(buf));
  
  sprintf(buf,"NICK %s\n",nickName);
  //skicka nickname 
  if ((recivedValue = send(clientSocket, buf, strlen(buf), 0)) == -1) 
  {
    perror("sendto:");
    exit(1);
  }
  
  memset(buf,0,sizeof(buf));
  //ta emot error eller ok
  if((recivedValue = recv(clientSocket,buf,sizeof(buf),0)) == -1)
  {
    perror("sendto:");
    exit(1);
  }
  

  if(strcmp(buf, "OK\n") == 0)
  {
    printf("recived OK: %s", buf);
  }
  else
  {
    printf("ERROR RECIVED: %s",buf);
    exit(4);
  }

  fd_set masterFds;
  FD_ZERO(&readfds);
  FD_ZERO(&masterFds);
  FD_SET(STDIN_FILENO,&masterFds);
  FD_SET(clientSocket, &masterFds);
  //loopStart
  while(1)
  {
    readfds = masterFds;
    memset(inputMsg,0,sizeof(inputMsg));
    memset(buf,0,sizeof(buf));
    recivedValue = select(clientSocket +1, &readfds,NULL,NULL,NULL);
    if(recivedValue == -1)
    {
      printf("Error with select");
      continue;
    }
    if(FD_ISSET(clientSocket, &readfds))
    {
      
      if ((recivedValue = recv(clientSocket, buf, sizeof(buf), 0)) == -1) 
      {
        perror("sendto:");
        exit(1);
      }
      else if(recivedValue == 0)
      {

        close(clientSocket);
        break;
      }
      char MSG[5];
      memset(MSG,0,sizeof(MSG));
      char *temp = strchr(buf,' ');
      char tempName[20];
      sscanf(buf,"%s %s",MSG, tempName);
      memset(printMsg,0,sizeof(printMsg));
      sprintf(printMsg,"%s",temp);
      
      for(int j = 0; j < strlen(printMsg) ; j++)
      {
        printMsg[j] = printMsg[j+1];
      }
      if(strcmp(tempName,nickName) != 0)
      {
        printf("%s",printMsg);
        fflush(stdout);
      }
    }

    if(FD_ISSET(STDIN_FILENO,&readfds))
    {
      
      memset(inputMsg,0,sizeof(inputMsg));
      fgets(inputMsg, 4096, stdin);
      if(strlen(inputMsg) > 255)
      {
        printf("to big message try again\n");
      }
      else
      {
        //skicka
        recivedValue = snprintf(buf,256,"MSG %s",inputMsg);
        if(recivedValue > 0)
        {                      
          #ifdef DEBUG        
          #endif
          if ((recivedValue = send(clientSocket, buf, strlen(buf), 0)) == -1) 
          {
            perror("sendto:");
            exit(1);
          }
        }
        
      }
      FD_CLR(clientSocket,&readfds);
    }
  }
  return 0;
}