#include "function.h"

void *workthread(void *socket);

CMysql* g_pMysql = CMysql::GetInstance();
CMyJson g_MyJson;

int main(int argc, char **argv)
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

	pthread_t thread[NUM_THREADS];
	int i = 0;

	  /* run server */
	while (1)
	{
		struct sockaddr_in client_addr;
		int client_socket;
		int nRet = 0;
		socklen_t length;

		    /* accept socket from client */
		if (i < NUM_THREADS)
		{
			length = sizeof(client_addr);
			client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &length);
			if (client_socket < 0)
			{
				printf("Server Accept Failed!\n");
				break;
			}
			printf("Server Accept Success!\n");
			printf("port : %u\n", ntohs(client_addr.sin_port));
			printf("client socket : [%d]\n", client_socket);

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
	int length = 0;
	int nRet = 0;
	time_t CurTimer, LastTimer;
	time(&LastTimer);

	unsigned long long ullClientID = 0;

	while (1)
	{
		time(&CurTimer);
		if (CurTimer - LastTimer > WU_KEEP_ALIVE_TIMEOUT)
		{
			printf("keep alive time out\n");
			break;
		}

		Header hd;
		bzero(&hd, sizeof(Header));
		length = Net_Receive(client_socket, (char*)&hd, sizeof(Header), 0);
		if (length < 0) break;
		//else if(length == 0) continue;

		if (memcmp(hd.szFlag, WU_HEADER_FLAG, WU_HEADER_FLAG_LEN) != 0)
		{
			printf("invalid header flag; [%0x],[%0x],[%0x],[%0x]\n",
				hd.szFlag[0],
				hd.szFlag[1],
				hd.szFlag[2],
				hd.szFlag[3]);
		  
			bool bFlag = false;
			if (SearchPacketHeader(client_socket, (char *)hd.szFlag, WU_HEADER_FLAG_LEN))
			{
				length = Net_Receive(client_socket, (char*)&hd.usCode, sizeof(Header) - WU_HEADER_FLAG_LEN, 0); 
				if (length == sizeof(Header) - WU_HEADER_FLAG_LEN)
					bFlag = true;
			}
			if (!bFlag) break;
		}

		unsigned int uiDataLen = hd.uiTotalLength - WU_HEADER_LEN;
		char *pDataBuff = NULL;
		if(uiDataLen != 0)
		{
			pDataBuff = (char *)malloc(uiDataLen);
			if (pDataBuff == NULL)
			{
				printf("Insufficient memory available\n");
				break;
			}
			bzero(pDataBuff, uiDataLen);

			length = Net_Receive(client_socket, pDataBuff, uiDataLen, 0);
			if (length < 0) break;
			//else if(length == 0) continue;
		}
		
		switch (hd.usCode)
		{
		case KEEP_ALIVE_REQ:
			{
				time(&LastTimer);
				Keep_Alive_Req_Function(client_socket, &hd, pDataBuff);
			}
			break;
		case LOGIN_REQ:
			{
				ullClientID = Login_Req_Function(client_socket, &hd, pDataBuff, hd.ullSrcId);
			}
			break;
		case GET_USER_LIST_REQ:
			{
				GetUserList_Req_Function(client_socket, ullClientID);
			}
			break;
		case TALK_WITH_USER_REQ:
			{
				TalkWithUser_Req_Function(client_socket, pDataBuff, ullClientID);
			}
			break;
		default:
			break;
		}

		free(pDataBuff);
	}

	close(client_socket);
}