#ifndef FUNCTION_H_
#define FUNCTION_H_

#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

#include "protocol.h"

#define HELLO_WORLD_SERVER_PORT 7878
#define LENGTH_OF_LISTEN_QUEUE 20
#define BUFFER_SIZE 1024
#define SOCKET_ERROR -1
#define NUM_THREADS 5

char tolower(char c) {
	if(c >= 'A' && c <= 'Z')
		c += 'a' - 'A';
	return c;
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
	int iMaxRevBytes = 10 * 1024;//Ҫ�㹻��
	for(int i =0; i < WU_HEADER_FLAG_LEN && iRevBytes < iMaxRevBytes;i++)
	{
		unsigned char uctmp = (unsigned char)'f';//��ʼ��������ν�ĸ��ַ�
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

int Net_Send(int client_socket, char* buf, int len, int flag)
{
	int nRet = NonblockingWrite(client_socket, WU_NETWORK_TIMEOUT);

	if(nRet == 0 || nRet == SOCKET_ERROR)
	{
		printf("client[%d] Nonblocking Write error, return[%d]\n", client_socket, nRet);
	}

	int length = send(client_socket, buf, len, flag);

	if(length == SOCKET_ERROR) 
	{
		printf("client[%d] send error\n", client_socket);
		return -1;
	}

	printf("client[%d] NonblockingWrite return : %d\n", client_socket, nRet);
	printf("client[%d] send length : %d\n", client_socket, length);

	return length;
}

int CreateHeader(Header* pHead, const WU_uint16_t &usCode,
	const WU_uint32_t &uiDataLen, const WU_uint64_t &ullDstId)
{
	memset(pHead, 0, sizeof(Header));
	memcpy(pHead->szFlag, WU_HEADER_FLAG, WU_HEADER_FLAG_LEN);
	pHead->usCode = usCode;
	pHead->uiTotalLength = uiDataLen + WU_HEADER_LEN;
	pHead->ullSrcId = WU_SERVER_ID;
	pHead->ullDstId = ullDstId;

	return 0;
}

int Keep_Alive_Rsp_Function(int client_socket, unsigned long long ullClientID, unsigned short usAliveSeq)
{
	Header hd;
	CreateHeader(&hd, KEEP_ALIVE_RSP, sizeof(KeepAliveRsp), ullClientID);

	Net_Send(client_socket, (char*)&hd, sizeof(hd), 0);

	KeepAliveRsp kar;
	kar.usAliveSeq = usAliveSeq;

	Net_Send(client_socket, (char*)&kar, sizeof(KeepAliveRsp), 0);

	return 0;
}

int Keep_Alive_Req_Function(int client_socket, Header* pHeader, const char* RecvBuffer)
{
	printf("KEEP_ALIVE_REQ\n");

	int nRet = 0;

	KeepAliveReq* pKeepAliveReq = (KeepAliveReq*)RecvBuffer;
	printf("alive seq : [%u]\n", pKeepAliveReq->usAliveSeq);

	Keep_Alive_Rsp_Function(client_socket, pHeader->ullSrcId, pKeepAliveReq->usAliveSeq);

	return nRet;
}

int Login_Rsp_Function(int client_socket, unsigned long long ullClientID, unsigned char ucResult)
{
	Header hd;
	CreateHeader(&hd, LOGIN_RSP, sizeof(LoginReq), ullClientID);

	Net_Send(client_socket, (char*)&hd, sizeof(hd), 0);

	LoginRsp lr;
	memset(&lr, 0, sizeof(lr));
	lr.ucResult = ucResult;
	switch(ucResult)
	{
	case 0:
		memcpy(lr.szReason, "success", 128);
		break;
	case 1:
		memcpy(lr.szReason, "password error", 128);
		break;
	case 2:
		memcpy(lr.szReason, "username error", 128);
		break;
	default:
		memcpy(lr.szReason, "unknown error", 128);
		break;
	}

	Net_Send(client_socket, (char*)&lr, sizeof(LoginRsp), 0);
}

int Login_Req_Function(int client_socket, Header* pHeader, const char* RecvBuffer)
{
	printf("LOGIN_REQ\n");

	int nRet = 0;

	LoginReq* pLoginReq = (LoginReq*)RecvBuffer;
	printf("username:%s, password:%s\n", pLoginReq->szUserName, pLoginReq->szPassWord);

	unsigned char ucResult = 0;
	if(memcmp(pLoginReq->szUserName, "wjw", strlen((const char*)pLoginReq->szUserName)) == 0)
	{
		if(memcmp(pLoginReq->szPassWord, "123456", strlen((const char*)pLoginReq->szPassWord)) == 0)
			printf("wjw login success\n");
		else 
		{
			printf("login failed, password error\n");
			ucResult = 1;
		}
	}
	else 
	{
		printf("login failed, username error\n");
		ucResult = 2;
	}

	unsigned long long ullClientID = 1;

	Login_Rsp_Function(client_socket, ullClientID, ucResult);

	return nRet;
}

#endif