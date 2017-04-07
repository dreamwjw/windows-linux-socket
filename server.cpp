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

int Net_Receive(int client_socket, char* buf, int len, int flag);
bool SearchPacketHeader(const int fd, char *buf,const int &iBufLen);
int NonblockingRead(int ifd, unsigned int uiTimeOut);
int NonblockingWrite(int ifd, unsigned int uiTimeOut);
int Login_Req_Function(const char* RecvBuffer);

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
	  printf("port : %u\n", ntohs(client_addr.sin_port));

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
	printf("client socket : [%d]\n", client_socket);
	int length = 0;
	int nRet = 0;

	while(1)
   {
	  Header hd;
	  bzero(&hd, sizeof(Header));
	  length = Net_Receive(client_socket, (char*)&hd, sizeof(Header), 0);
	  if(length < 0) break; 

	  if(memcmp(hd.szFlag, WU_HEADER_FLAG, WU_HEADER_FLAG_LEN) != 0)
	  {
		  printf("invalid header flag; [%0x],[%0x],[%0x],[%0x]\n", hd.szFlag[0], hd.szFlag[1],
			  hd.szFlag[2], hd.szFlag[3]);
		  
		  bool bFlag = false;
		  if(SearchPacketHeader(client_socket, (char *)hd.szFlag, WU_HEADER_FLAG_LEN))
		  {
			  length = Net_Receive(client_socket, (char*)&hd.usCode, sizeof(Header) - WU_HEADER_FLAG_LEN, 0); 
			  if(length == sizeof(Header) - WU_HEADER_FLAG_LEN)
				  bFlag = true;
		  }
		  if(!bFlag) break;
	  }

	  unsigned int uiDataLen = hd.uiTotalLength - WU_HEADER_LEN;
	  char *pDataBuff = (char *)malloc(uiDataLen);
	  if(pDataBuff == NULL)
	  {
		  printf("Insufficient memory available\n");
		  break;
	  }
	  bzero(pDataBuff, uiDataLen);

	  length = Net_Receive(client_socket, pDataBuff, uiDataLen, 0);
	  if(length < 0) break; 

	  switch(hd.usCode)
	  {
	  case LOGIN_REQ:
		  Login_Req_Function(pDataBuff);
		  break;
	  default:
		  break;
	  }

	  free(pDataBuff);
   }

   close(client_socket);
}

int Login_Req_Function(const char* RecvBuffer)
{
	int nRet = 0;

	LoginReq* pLoginReq = (LoginReq*)RecvBuffer;
	printf("username:%s, password:%s\n", pLoginReq->szUserName, pLoginReq->szPassWord);

	if(memcmp(pLoginReq->szUserName, "wjw", strlen((const char*)pLoginReq->szUserName)) == 0)
	{
		if(memcmp(pLoginReq->szPassWord, "123456", strlen((const char*)pLoginReq->szPassWord)) == 0)
			printf("wjw login success\n");
		else 
		{
			printf("login failed, password error\n");
			nRet = -1;
		}
	}
	else 
	{
		printf("login failed, username error\n");
		nRet = -2;
	}

	return nRet;
}

bool SearchPacketHeader(const int fd, char *buf,const int &iBufLen)
{
	if(iBufLen < WU_HEADER_FLAG_LEN)
	{
		printf("Search PacketHeader error iBufflen [%d] not enough,iClientFd[%d]\n",iBufLen,fd);
		return false;
	}

	unsigned char ucPacketHeader[WU_HEADER_FLAG_LEN+1] = {0};
	memcpy(ucPacketHeader, WU_HEADER_FLAG, WU_HEADER_FLAG_LEN);
	int iRevBytes = 0;
	int iMaxRevBytes = 10 * 1024;//要足够大
	for(int i =0; i < WU_HEADER_FLAG_LEN && iRevBytes < iMaxRevBytes;i++)
	{
		unsigned char uctmp = (unsigned char)'f';//初始化，无所谓哪个字符
		int iRes = recv(fd,(char *)&uctmp,1,0);
		if(iRes == -1)
		{
			printf("Search PacketHeader error iRes :%d,iClientFd[%d]\n",iRes,fd);
			return false;
		}
		if(ucPacketHeader[i] == uctmp)
		{
			iRevBytes++;
			continue;
		}
		else
		{
			i = 0;
			iRevBytes++;
		}
	}
	if(iRevBytes < iMaxRevBytes)
	{
		printf("ok find Packet Header[%s],iRevByes[%d],iClientFd[%d]\n",ucPacketHeader,iRevBytes,fd);
		memcpy(buf,ucPacketHeader,WU_HEADER_FLAG_LEN);
		return true;
	}
	else
	{
		printf(" cannot Search PacketHeader[%s],iRevBytes[%d],iClientFd[%d]\n",ucPacketHeader,iRevBytes,fd);
		return false;
	}
}

int NonblockingRead(int ifd, unsigned int uiTimeOut)
{
	if(ifd < 0)
	{
		return -2;//select failed would retuan SOCKET_ERROR(-1)
	}

	fd_set read_set;
	FD_ZERO(&read_set);
	FD_SET(ifd, &read_set);

	struct timeval tv;
	tv.tv_sec = uiTimeOut / 1000;
	tv.tv_usec = 1000 * (uiTimeOut % 1000);

	int iMaxFd = ifd + 1;
	int iRes = select(iMaxFd, &read_set, NULL, NULL, &tv);

	return iRes;
}

int NonblockingWrite(int ifd, unsigned int uiTimeOut)
{
	if (ifd < 0)
	{
		return -2;
	}

	fd_set write_set;
	FD_ZERO(&write_set);
	FD_SET(ifd, &write_set);

	struct timeval tv;
	tv.tv_sec = uiTimeOut / 1000;
	tv.tv_usec = 1000 * (uiTimeOut % 1000);

	int iMaxFd = ifd + 1;
	int iRes = select(iMaxFd, NULL, &write_set, NULL, &tv);

	return iRes;
}

int Net_Receive(int client_socket, char* buf, int len, int flag)
{
	int nRet = NonblockingRead(client_socket, WU_NETWORK_TIMEOUT);

	if(nRet == 0 || nRet == SOCKET_ERROR)
	{
		printf("client[%d] Nonblocking Read error, return[%d]\n", client_socket, nRet);
	}

	int length = recv(client_socket, buf, len, flag);

	if(length == 0) 
	{
		printf("client[%d] exit\n", client_socket);
		return -1;
	}

	if(length < 0)
	{
		printf("client[%d] exception exit\n", client_socket);
		return -2;
	}

	printf("client[%d] NonblockingRead return : %d\n", client_socket, nRet);
	printf("client[%d] recv length : %d\n", client_socket, length);

	return length;
}