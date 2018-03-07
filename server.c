#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <err.h>
#include <signal.h>

#define PORT 12345
#define MAXCLIENTS 10

int socketfd;

void exitHandler(int e)
{
  close(socketfd);
  printf("Server shutting down...\n");
  exit(0);
}

void *
start_client(void *x)
{
  int rb;
  int clientfd = *(int *)x;
  char str[100];
  while(1)
  {
    if ((rb = read(clientfd, str, 100)) == -1)
    {
      err(1, "Error reading message");
    }
    printf("Echoing back from %d: %s\n", clientfd, str);
    if (write(clientfd, str, strlen(str) + 1) == -1)
    {
      err(1, "Error writing message");
    }
    if (!strcmp(str, ".exit"))
      break;
  }
  printf("Client exiting\n");
  close(clientfd);
  return 0;
}

int
main(int argc, char **argv)
{

  struct sockaddr_storage ca;
  int clientfd;
  struct addrinfo *it;
  struct addrinfo *addresses;
  struct addrinfo hi;
  int option = 1;

  signal(SIGINT, exitHandler);

  memset(&hi, 0, sizeof (hi));
  hi.ai_family = AF_UNSPEC;
  hi.ai_socktype = SOCK_STREAM;
  hi.ai_flags = AI_PASSIVE;

  if (getaddrinfo(NULL, "22000", &hi, &addresses))
  {
    err(1, "Error getting address info");
  }

  for (it = addresses; it != NULL; it = it->ai_next)
  {
    socketfd = socket(it->ai_family, it->ai_socktype, it->ai_protocol);
    if (!bind(socketfd, it->ai_addr, it->ai_addrlen))
    {
      break;
    }
  }

  freeaddrinfo(addresses);
  if (it == NULL)
  {
    close(socketfd);
    err(1, "None of the addresses could bind to socket");
  }

  if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) == -1)
  {
    err(1, "Error setting socket options");
  }

  if (listen(socketfd, MAXCLIENTS) == -1)
  {
    err(1, "Error setting listener");
  }
  while (1)
  {
    unsigned int sz = sizeof(ca);
    /* communication with the client */

    clientfd = accept(socketfd, (struct sockaddr *)&ca, &sz);
    if (clientfd == -1)
    {
      err(1, "Error accepting client");
    }
    printf("Client accepted\n");
    int *sock = malloc(1);
    *sock = clientfd;
    pthread_t t;
    pthread_create(&t, NULL, start_client, (void *)&clientfd);
    // pthread_join(t, NULL);
  }


}
