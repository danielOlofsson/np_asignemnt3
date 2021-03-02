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
  printf("Got %s split into %s and %s \n",org, hoststring,portstring);


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
  
  printf("Testing nicknames. \n");
  
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
        
        printf("%s is not accepted. Exiting\n",argv[i]);
        exit(1);
        
      }
    } 
    else 
    {
      
      printf("%s is too long (%ld vs 12 chars).Exiting\n", argv[i], strlen(argv[i]));
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
  char inputMsg[252];
  memset(buf,0,sizeof(buf));
 

  // ta emot hello
  if((recivedValue = recv(clientSocket,buf,sizeof(buf),0)) == -1)
  {
    perror("sendto:");
    exit(1);
  }
  printf("When reciving hello: %s", buf);
  memset(buf,0,sizeof(buf));
  
  sprintf(buf,"NICK %s\n",nickName);
  //skicka nickname 
  if ((recivedValue = send(clientSocket, buf, strlen(buf), 0)) == -1) 
  {
    perror("sendto:");
    exit(1);
  }
  printf("client sent %d bytes",recivedValue);
  memset(buf,0,sizeof(buf));
  //ta emot error eller ok
  if((recivedValue = recv(clientSocket,buf,sizeof(buf),0)) == -1)
  {
    perror("sendto:");
    exit(1);
  }
  printf("When reciving ok or error: %s", buf);

  if(strcmp(buf, "OK\n") == 0)
  {
    printf("recived OK: %s", buf);
  }
  else
  {
    printf("ERROR RECIVED: %s",buf);
  }
  
  //loopStart
  while(1)
  {
    FD_SET(STDIN,&readfds);
    FD_SET(clientSocket, &readfds);
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
      printf("Message recived: %s",buf);
    }

    if(FD_ISSET(STDIN,&readfds))
    {
      
      fgets(inputMsg,252,stdin);
      
      snprintf(buf,256,"MSG %s",inputMsg);
      printf("BUF BEFORE SENDING: %s",buf);
      if ((recivedValue = send(clientSocket, buf, strlen(buf), 0)) == -1) 
      {
        perror("sendto:");
        exit(1);
      }
      printf("sent: %d bytes\n",recivedValue);
    }
  }
  return 0;
}