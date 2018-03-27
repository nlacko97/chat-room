#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <err.h>
#include <pthread.h>

/*
 -------------------------------------------------
  -> Nagy Laszlo <-
  * ChatRoom Application
  * Server is running on localhost, on port 22000
  * Clients can connect to the server simultaneously and send messages
  * The application being a group chat, the messages are sent to everyone connected
  * to the server along with the senders name
  * The client application has the connection parameters hardcoded, so they don't have to be
  * provided externally

  ------------------------------------------------
  *
*/

#define PORT "22000"
#define HOST "localhost"
#define BUFSIZE 1024
#define MAX_PERSON_NAME 100
int sockfd;

void
exitHandler(int e)
{
  write(sockfd, ".exit", 6);
  close(sockfd);
  exit(0);
}

void
signalExit()
{
  if (kill(getpid(), SIGINT) == -1)
  {
    printf("Error sending exit signal");
  }
}

void *
readFromServer(void *x)
{
  int fd = *(int *)x;
  char *recvline = malloc(BUFSIZE * sizeof(char));
  while(1)
  {
    bzero(recvline, BUFSIZE);
    read(fd, recvline, BUFSIZE);
    if (!strcmp(recvline, ".exit"))
      break;
    printf("\r%s\n", recvline);
  }
  free(recvline);
  signalExit();
  return 0;
}


int
main(int argc, char **argv)
{

  if (signal(SIGINT, exitHandler) == SIG_ERR)
  {
    err(1, "Error setting signal handler");
  }

  // int fd;
  struct addrinfo *it;
  struct addrinfo *addresses;
  struct addrinfo hi;

  memset(&hi, 0, sizeof (hi));
  hi.ai_family = AF_UNSPEC;
  hi.ai_socktype = SOCK_STREAM;

  if (getaddrinfo(HOST, PORT, &hi, &addresses))
  {
    perror("Error getting address info");
    signalExit();
  }

  for (it = addresses; it != NULL; it = it->ai_next)
  {
    sockfd = socket(it->ai_family, it->ai_socktype, it->ai_protocol);
    if (connect(sockfd, (struct sockaddr *)it->ai_addr, it->ai_addrlen) == 0)
    {
      break;
    }
  }

  freeaddrinfo(addresses);
  if (it == NULL)
  {
    if (close(sockfd) == -1)
    {
      warn("Error closing socketfd");
    }
    err(1, "None of the addresses could connect to socket");
  }
  // inet_pton(AF_INET, "::1", &(servaddr.sin_addr));

  int b;
  char *sendline = (char*)malloc(BUFSIZE * sizeof(char));
  bzero(sendline, BUFSIZE);
  printf("*******************************************************\n");
  printf("--------------->>>>>>>>>ChatRoom<<<<<<<<<--------------\n");
  printf("Succesfully connected to server!\n");
  printf("Please write in your name(maximum 100 characters)\n");
  b = read(0, sendline, BUFSIZE);
  if (b == -1)
  {
    perror("Error reading username");
    signalExit();
  }
  if (b - 1 < MAX_PERSON_NAME)
    b = b - 1;
  else
    b = MAX_PERSON_NAME - 1;
  sendline[b] = '\0';
  if (write(sockfd, sendline, b ) == -1)
  {
    perror("Error writing username");
    signalExit();
  }
  printf("\tlen:%d\n",b);
  printf("\n\t Welcome %s! Now you can write and receive messages from the group!\n\n", sendline);
  memset(sendline, (char)0, BUFSIZE);
  // sendline[0] = '\0';
  pthread_t t;
  if (pthread_create(&t, NULL, readFromServer, (void *)&sockfd))
  {
    perror("Could not create listener thread");
    signalExit();
  }
  sendline = (char*)realloc(sendline, BUFSIZE*sizeof(char));
  while(1)
  {
    bzero(sendline, BUFSIZE);
    if (read(0, sendline, BUFSIZE) == -1)
    {
      signalExit();
    }
    sendline[strlen(sendline) - 1] = '\0';
    if (write(sockfd, sendline, strlen(sendline) + 1) == -1)
    {
      perror("Write error");
      signalExit();
    }


  }
  close(sockfd);

}
