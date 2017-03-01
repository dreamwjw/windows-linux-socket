#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <mysql/mysql.h>

#define HELLO_WORLD_SERVER_PORT 4000
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

bool select(const char *strSelect, MYSQL *conn, int client_socket) {
   char SendBuffer[BUFFER_SIZE] = {0};
   int nRet = 0;

   MYSQL_RES *res;
   MYSQL_FIELD *fields;
   MYSQL_ROW row;

   if (mysql_query(conn, strSelect))
   {
     printf("%s\n", mysql_error(conn));
     sprintf(SendBuffer, "%s\n", mysql_error(conn));
     nRet = send(client_socket, SendBuffer, BUFFER_SIZE, 0);
     if(nRet == SOCKET_ERROR)
     {
       printf("Send Info Error!\n");
       return false;
     }
     printf("Send Info Success!\n");
     return false;
   }

   res = mysql_use_result(conn);

   int nFields = mysql_num_fields(res);

   char TempBuffer[MIN_BUFFER_SIZE] = {0};

   fields = mysql_fetch_fields(res);

   for (int i = 0; i < nFields; i++)
   {
     sprintf(TempBuffer, "%s ", fields[i].name);
     strcat(SendBuffer, TempBuffer);
   }
   strcat(SendBuffer, "\n");

   while ((row = mysql_fetch_row(res)) != NULL)
   {
     for (int i = 0; i < nFields; i++)
     {
       sprintf(TempBuffer, "%s ", row[i]);
       strcat(SendBuffer, TempBuffer);
     }
     strcat(SendBuffer, "\n");
   }

   nRet = send(client_socket, SendBuffer, BUFFER_SIZE, 0);
   if(nRet == SOCKET_ERROR)
   {
     printf("Send Info Error!\n");
   }
   else printf("Send Info Success!\n");

   mysql_free_result(res);

   return true;
}

bool noselect(char *strSQL, MYSQL *conn, int client_socket) {
  char SendBuffer[BUFFER_SIZE] = {0};
  int nRet = 0;

  if (mysql_query(conn, strSQL))
  {
      sprintf(SendBuffer, "%s\n", mysql_error(conn));
      printf("%s", SendBuffer);

      nRet = send(client_socket, SendBuffer, BUFFER_SIZE, 0);
      if(nRet == SOCKET_ERROR)
      {
        printf("Send Info Error!\n");
        return false;
      }
      printf("Send Info Success!\n");

      return false;
  }

  int nAffectedRow =  mysql_affected_rows(conn);
  sprintf(SendBuffer, "%d rows affected\n", nAffectedRow);

  nRet = send(client_socket, SendBuffer, BUFFER_SIZE, 0);
  if(nRet == SOCKET_ERROR)
  {
     printf("Send Info Error!\n");
  }
  else printf("Send Info Success!\n");

  return true;
}

bool nosql(char *strNoSQL, int client_socket) {
  char SendBuffer[BUFFER_SIZE] = {0};
  int nRet = 0;

  sprintf(SendBuffer, "%s is no SQL", strNoSQL);

  nRet = send(client_socket, SendBuffer, BUFFER_SIZE, 0);
  if(nRet == SOCKET_ERROR)
  {
     printf("Send Info Failed\n");
     return false;
  }
  printf("Send Info Success\n");

  return true;
}

void *socketthread(void *socket)
{
   printf("welcome new client!\n");

   int client_socket = (int)(*((int*)socket));

   char RecvBuffer[BUFFER_SIZE] = {0}, SendBuffer[BUFFER_SIZE] = {0};
   socklen_t length = 0;
   int nRet = 0;

   MYSQL *conn;
   char server[] = "localhost", user[] = "root", password[] = "Wjw7132131",
        database[] = "test";

   /*conn = mysql_init(NULL);
   if (!mysql_real_connect(conn, server,user, password, database, 0, NULL, 0))
   {
       printf("%s\n", mysql_error(conn));
       close(client_socket);
       return (void*)0;
   }*/

   while(1)
   {
      bzero(RecvBuffer, BUFFER_SIZE);
      length = recv(client_socket, RecvBuffer, BUFFER_SIZE, 0);
      if(length < 0)
      {
        printf("Server Receive Data Failed!\n");
        break;
      }
      printf("Server Receive Data Success!\n");

      if(RecvBuffer[0] == 'q')
      {
        printf("Client exit!\n");
        break;
      }

      printf("Client say: %s\n", RecvBuffer);

      /*switch(tolower(RecvBuffer[0]))
      {
      case 's':
         if(!select(RecvBuffer, conn, client_socket))
         {
           printf("Select Failed!\n");
         }
         else printf("Select Success!\n");
         break;
      case 'i':
      case 'u':
      case 'd':
         if(!noselect(RecvBuffer, conn, client_socket))
         {
           printf("NoSelect Failed!\n");
         }
         else printf("NoSelect Success!\n");
         break;
      default:
         if(!nosql(RecvBuffer, client_socket))
         {
           printf("NoSQL Failed!\n");
         }
         else printf("NoSQL Success!\n");
         break;
      }*/
   }

   mysql_close(conn);

   close(client_socket);
}
int main(int argc, char **argv) {
  //printf("Hello world/n");
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

      pthread_create(&thread[i], NULL, socketthread, (void *)&client_socket);
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
