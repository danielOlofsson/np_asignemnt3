#include <stdio.h>
#include <stdlib.h>
/* You will to add includes here */
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
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

  int listenSocket;
  int acceptFd;

  struct sockaddr_storage clientAddr;
  socklen_t addrLenght;

  char buf[256];
  int nbytes;
  char remoteIP[INET6_ADDRSTRLEN];

  int yes = 1;
  int recivedValue;
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
  printf("Before loop\n");
  while (1)
  {
    
  }
  
  return(0);
}
