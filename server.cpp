#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#include "protocol.h"

#define HELLO_WORLD_SERVER_PORT 7878
#define LENGTH_OF_LISTEN_QUEUE 20
#define BUFFER_SIZE 1024
#define MIN_BUFFER_SIZE 50
#define SOCKET_ERROR -1
#define NUM_THREADS 5

char tolower(char c) {
  if(c >= 'A' && c <= 'Z')
    c += 'a' - 'A';
  return c;
}

void *workthread(void *socket);

int main(int argc, char **argv) {
  struct sockaddr_in server_addr;
  int server_socket;
  int opt = 1;

  bzero(&server_addr, sizeof(server_addr));

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htons(INADDR_ANY);
  server_addr.sin_port = htons(HELLO_WORLD_SERVER_PORT);

  /* create a socket */
  server_socket = socket(PF_INET, SOCK_STREAM, 0);
  if(server_socket < 0)
  {
    printf("Create Socket Failed!\n");
    exit(1);
  }
  printf("Create Socket Success!\n");

  /* bind socket to a specified address */
  setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  if(bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)))
  {
    printf("Server Bind Port : %d Failed!\n", HELLO_WORLD_SERVER_PORT);
    exit(1);
  }
  printf("Server Bind Port : %d Success!\n", HELLO_WORLD_SERVER_PORT);

  /* listen a socket */
  if(listen(server_socket, LENGTH_OF_LISTEN_QUEUE))
  {
    printf("Server Listen Failed!\n");
    exit(1);
  }
  printf("Server Listen Success!\n");

  pthread_t thread[NUM_THREADS];
  int i = 0;

  /* run server */
  while(1)
  {
    struct sockaddr_in client_addr;
    int client_socket;
    int nRet = 0;
    socklen_t length;

    /* accept socket from client */
    if(i < NUM_THREADS)
    {
      length = sizeof(client_addr);
      client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &length);
      if(client_socket < 0)
      {
        printf("Server Accept Failed!\n");
        break;
      }
      printf("Server Accept Success!\n");

      pthread_create(&thread[i], NULL, workthread, (void *)&client_socket);
      ++i;
    }
    else
    {
      for (int j = 0; j < NUM_THREADS; j++)
      {
        pthread_join(thread[j], NULL);
      }
      i = 0;
    }
  }

  close(server_socket);
  return 0;
}

void *workthread(void *socket)
{
	int client_socket = (int)(*((int*)socket));

	char RecvBuffer[BUFFER_SIZE] = {0}, SendBuffer[BUFFER_SIZE] = {0};
	socklen_t length = 0;
	int nRet = 0;

	while(1)
   {
      bzero(RecvBuffer, BUFFER_SIZE);
      length = recv(client_socket, RecvBuffer, BUFFER_SIZE, 0);
      if(length == 26)
	  {
		  Header* pHeader = (Header*)RecvBuffer;
		  if(memcmp(pHeader->szFlag, WU_HEADER_FLAG, WU_HEADER_FLAG_LEN) == 0)
		  {

		  }
	  }
   }

   close(client_socket);
}