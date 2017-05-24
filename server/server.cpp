#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <vector>
using namespace std;

#include "protocol.h"
#include "NetWork.h"
#include "mysql.h"
#include "struct.h"

#define HELLO_WORLD_SERVER_PORT 7878
#define LENGTH_OF_LISTEN_QUEUE 1000

extern CMysql* g_pMysql;
extern CNetWork g_NetWork;

int MysqlInit();
int SocketInit();

bool g_bExit = false;
void* ThreadAccept(void* socket);
void* ThreadSend(void* socket);
void* ThreadRecv(void* socket);

int AddClientSocket(int client_socket);
bool IsTimeOut(int client_socket, time_t LastTimer);
int ClientExit(int i);
int RecvFromClient(int client_socket, Header& hd, char*& pDataBuffer);
int DoWorkForRecv(int i, int client_socket, Header& hd, char* pDataBuffer);

vector<ClientSocketStruct> g_vecClientSocket;

int main(int argc, char **argv)
{
	if(MysqlInit() != 0) return -1;

	int server_socket = SocketInit();
	if(server_socket <= 0) return -2;

	pthread_t ta, ts, tr;
	pthread_create(&ta, NULL, ThreadAccept, (void *)&server_socket);
	pthread_create(&ts, NULL, ThreadSend, NULL);
	pthread_create(&tr, NULL, ThreadRecv, NULL);
	
	while(true)
	{
		char c;
		scanf("%c", &c);
		if(c == 'q') break;
	}

	g_bExit = true;
	pthread_join(ta, NULL);
	pthread_join(ts, NULL);
	pthread_join(tr, NULL);
	
	close(server_socket);

	return 0;
}

int MysqlInit()
{
	g_pMysql = CMysql::GetInstance();
	if(g_pMysql == NULL)
	{
		printf("mysql connect failed");
		return -1;
	}

	printf("mysql connect success");

	return 0;
}

int SocketInit()
{
	struct sockaddr_in server_addr;
	int server_socket;
	int opt = 1;

	bzero(&server_addr, sizeof(server_addr));

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htons(INADDR_ANY);
	server_addr.sin_port = htons(HELLO_WORLD_SERVER_PORT);

	/* create a socket */
	server_socket = socket(PF_INET, SOCK_STREAM, 0);
	if (server_socket < 0)
	{
		printf("Create Socket Failed!\n");
		exit(1);
	}
	printf("Create Socket Success!\n");

	/* bind socket to a specified address */
	setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)))
	{
		printf("Server Bind Port : %d Failed!\n", HELLO_WORLD_SERVER_PORT);
		exit(1);
	}
	printf("Server Bind Port : %d Success!\n", HELLO_WORLD_SERVER_PORT);

	/* listen a socket */
	if (listen(server_socket, LENGTH_OF_LISTEN_QUEUE))
	{
		printf("Server Listen Failed!\n");
		exit(1);
	}
	printf("Server Listen Success!\n");

	return server_socket;
}

void* ThreadAccept(void* socket)
{
	int server_socket = (int)(*((int*)socket));

	while(!g_bExit)
	{
		struct sockaddr_in client_addr;
		int client_socket;
		socklen_t length;

		length = sizeof(client_addr);
		client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &length);
		if (client_socket < 0)
		{
			printf("Server Accept Failed!\n");
			continue;
		}

		printf("Server Accept Success!\n");
		printf("port : %u\n", ntohs(client_addr.sin_port));
		printf("client socket : [%d]\n", client_socket);

		int nRet = g_NetWork.setSocketNonBlock(client_socket, 1);

		if(nRet == -1)
		{
			close(client_socket);
		}
		else
		{
			AddClientSocket(client_socket);
		}
	}
}

void* ThreadSend(void* socket)
{

}

void* ThreadRecv(void* socket)
{
	while(!g_bExit)
	{
		int nSize = g_vecClientSocket.size();
		for(int i = 0; i < nSize; i++)
		{
			int client_socket = g_vecClientSocket[i].m_nClientSokcet;
			time_t LastTimer = g_vecClientSocket[i].m_LastTimer;
			if(client_socket == 0) continue;

			if(IsTimeOut(client_socket, LastTimer))
			{
				ClientExit(i);
				continue;
			}

			Header hd;
			memset(&hd, 0, sizeof(Header));
			char* pDataBuffer = NULL;
			int nRet = RecvFromClient(client_socket, hd, pDataBuffer);
			if(nRet == 0)
			{			
				DoWorkForRecv(i, client_socket, hd, pDataBuffer);
			}
			else if(nRet == -1)
			{
				ClientExit(i);
				continue;
			}
			else if(nRet == -2)
			{
				continue;
			}
		}
	}
	
}

int AddClientSocket(int client_socket)
{
	int nSize = g_vecClientSocket.size();
	int i = 0;
	for(; i < nSize; i++)
	{
		if(g_vecClientSocket[i].m_nClientSokcet == client_socket) break;
	}
	if(i < nSize) printf("client[%d] has exist", client_socket);
	else
	{
		int j = 0;
		for(; j < nSize; j++)
		{
			if(g_vecClientSocket[j].m_nClientSokcet == 0)
			{
				g_vecClientSocket[j].m_nClientSokcet = client_socket;
				time(&g_vecClientSocket[j].m_LastTimer);
				break;
			}
		}
		if(j >= nSize) 
		{
			time_t LastTimer;
			time(&LastTimer);
			ClientSocketStruct css(client_socket, LastTimer);
			g_vecClientSocket.push_back(css);
		}
	}

	return 0;
}

bool IsTimeOut(int client_socket, time_t LastTimer)
{
	time_t CurTimer;
	time(&CurTimer);
	if (CurTimer - LastTimer > WU_KEEP_ALIVE_TIMEOUT)
	{
		printf("client[%d] keep alive time out\n", client_socket);
		return true;
	}

	return false;
}

int ClientExit(int i)
{
	close(g_vecClientSocket[i].m_nClientSokcet);

	g_pMysql->mysql_DeleteOnlineUser(g_vecClientSocket[i].m_nClientSokcet);

	g_vecClientSocket[i].m_nClientSokcet = 0;
	g_vecClientSocket[i].m_LastTimer = 0;

	return 0;
}

int RecvFromClient(int client_socket, Header& hd, char*& pDataBuffer)
{
	int length = g_NetWork.Net_Receive(client_socket, (char*)&hd, sizeof(Header), 0);
	if (length < 0) return -1;
	else if(length == 0) return -2;

	if (memcmp(hd.szFlag, WU_HEADER_FLAG, WU_HEADER_FLAG_LEN) != 0)
	{
		printf("invalid header flag; [%0x],[%0x],[%0x],[%0x]\n",
			hd.szFlag[0],
			hd.szFlag[1],
			hd.szFlag[2],
			hd.szFlag[3]);

		bool bFlag = false;
		if(g_NetWork.SearchPacketHeader(client_socket, (char *)hd.szFlag, WU_HEADER_FLAG_LEN))
		{
			length = g_NetWork.Net_Receive(client_socket, (char*)&hd.usCode, sizeof(Header) - WU_HEADER_FLAG_LEN, 0); 
			if (length == sizeof(Header) - WU_HEADER_FLAG_LEN)
				bFlag = true;
		}
		if(!bFlag) return -1;
	}

	unsigned int uiDataLen = hd.uiTotalLength - WU_HEADER_LEN;
	if(uiDataLen != 0)
	{
		pDataBuffer = (char *)malloc(uiDataLen);
		if (pDataBuffer == NULL)
		{
			printf("Insufficient memory available\n");
			return -2;
		}
		bzero(pDataBuffer, uiDataLen);

		length = g_NetWork.Net_Receive(client_socket, pDataBuffer, uiDataLen, 0);
		if (length < 0) return -1;
		else if(length == 0) return -2;
	}

	return 0;
}

int DoWorkForRecv(int i, int client_socket, Header& hd, char* pDataBuffer)
{
	switch (hd.usCode)
	{
	case KEEP_ALIVE_REQ:
		{
			time_t LastTimer;
			time(&LastTimer);
			g_vecClientSocket[i].m_LastTimer = LastTimer;
			g_NetWork.Keep_Alive_Req_Function(client_socket, &hd, pDataBuffer);
		}
		break;
	case LOGIN_REQ:
		{
			g_NetWork.Login_Req_Function(client_socket, &hd, pDataBuffer, hd.ullSrcId);
		}
		break;
	case GET_USER_LIST_REQ:
		{
			g_NetWork.GetUserList_Req_Function(client_socket);
		}
		break;
	case TALK_WITH_USER_REQ:
		{
			g_NetWork.TalkWithUser_Req_Function(client_socket, pDataBuffer);
		}
		break;
	default:
		break;
	}

	free(pDataBuffer);
}