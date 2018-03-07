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
#include <sys/select.h>

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


#define MAX_PERSON_NAME 30
#define MAX_CLIENTS 20
#define BUFSIZE 256
#define PORT "22000"

struct Client
{
  char name[MAX_PERSON_NAME];
  int socket;
};

struct Client clients[MAX_CLIENTS];

int socketfd;

void
exitHandler(int e)
{
  if (close(socketfd) == -1)
  {
    warn("Error closing socketfd");
  }
  printf("Server shutting down...\n");
  exit(0);
}

int
createServerSocket()
{
  struct addrinfo *it;
  struct addrinfo *addresses;
  struct addrinfo hi;

  memset(&hi, 0, sizeof (hi));
  hi.ai_family = AF_UNSPEC;
  hi.ai_socktype = SOCK_STREAM;
  hi.ai_flags = AI_PASSIVE;

  if (getaddrinfo(NULL, PORT, &hi, &addresses))
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
    if (close(socketfd) == -1)
    {
      warn("Error closing socketfd");
    }
    err(1, "None of the addresses could bind to socket");
  }

  return socketfd;
}

int
main(int argc, char const *argv[])
{

  if (signal(SIGINT, exitHandler) == SIG_ERR)
  {
    err(1, "Error setting signal handler");
  }

  fd_set connections;
  struct sockaddr_storage ca;
  int clientfd;

  // int clients[MAX_CLIENTS];
  int max_c;
  char *buf = (char*)malloc(BUFSIZE*sizeof(char*));

  for(int i = 0;  i < MAX_CLIENTS; i++)
  {
    strcpy(clients[i].name, "");
    clients[i].socket = 0;
  }

  socketfd = createServerSocket();



  if (listen(socketfd, MAX_CLIENTS) == -1)
  {
    if (close(socketfd) == -1)
    {
      warn("Error closing socketfd");
    }
    err(1, "Error setting listener");
  }

  int activity;

  while(1)
  {
    FD_ZERO(&connections);
    FD_SET(socketfd, &connections);
    max_c = socketfd;
    bzero(buf, BUFSIZE);
    for(int i = 0; i < MAX_CLIENTS; i++)
    {
      if (clients[i].socket)
      {
        FD_SET(clients[i].socket, &connections);
      }
      if (clients[i].socket > max_c)
      {
        max_c = clients[i].socket;
      }
    }

    if ((activity = select(max_c + 1, &connections, NULL, NULL, NULL)) == -1)
    {
      if (close(socketfd) == -1)
      {
        warn("Error closing socketfd");
      }
      err(1, "Select error");
    }

    if (FD_ISSET(socketfd, &connections))
    {
      unsigned int sz = sizeof(ca);
      printf("Incoming connection\n");
      if ((clientfd = accept(socketfd, (struct sockaddr *)&ca, &sz)) == -1)
      {
        if (close(socketfd) == -1)
        {
          warn("Error closing socketfd");
        }
        err(1, "Error accepting new client");
      }
      printf("Accepted connection\n");
      for(int i = 0; i < MAX_CLIENTS; i++)
      {
        if (!clients[i].socket)
        {
          clients[i].socket = clientfd;
          read(clientfd, buf, MAX_PERSON_NAME);
          strcpy(clients[i].name, buf);
          break;
        }
      }
     }

     for(int i = 0; i < MAX_CLIENTS; i++)
     {
       int s = clients[i].socket, bytes;
       if (FD_ISSET(s, &connections))
       {
         printf("Incoming message from %s\n", clients[i].name);
         bytes = read(s, buf, BUFSIZE);
         {
           if (bytes == -1)
           {
             if (close(socketfd) == -1)
             {
               warn("Error closing socketfd");
             }
             err(1, "Error reading message");
           }
           if (!strcmp(buf, ".exit"))
           {
             write(s, buf, strlen(buf) + 1);
             printf("Client exiting\n");
             if (close(s) == -1)
             {
               if (close(socketfd) == -1)
               {
                 warn("Error closing socketfd");
               }
               err(1, "Error closing socket");
             }
             clients[i].socket = 0;
             bzero(clients[i].name, MAX_PERSON_NAME);
           }
           else
           {
             char *msg = (char*)malloc(BUFSIZE*sizeof(char*));
             sprintf(msg, "[%s] -> %s", clients[i].name, buf);
             msg[strlen(msg)] = '\0';
             //  printf("***\n");
             for(int j = 0; j < MAX_CLIENTS; j++)
             {
               if (clients[j].socket)
               {
                 printf("Sending to %s\n", clients[j].name);
                 if (write(clients[j].socket, msg, strlen(msg) + 1) == -1)
                 {
                   if (close(socketfd) == -1)
                   {
                     warn("Error closing socketfd");
                   }
                   err(1, "Error writing message");
                 }
               }
             }
           }
         }
       }
     }

  }


  return 0;
}
