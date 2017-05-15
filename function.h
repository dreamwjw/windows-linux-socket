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
#include "mysql.h"
#include "MyJson.h"

extern CMysql* g_pMysql;
extern CMyJson g_MyJson;

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

	if(nRet < 0)
	{
		printf("client[%d] Nonblocking Read error, return[%d]\n", client_socket, nRet);
	}

	int length = recv(client_socket, buf, len, flag);

	if(length == 0)
	{
		printf("client[%d] exit\n", client_socket);
		g_pMysql->mysql_DeleteOnlineUsers(client_socket);
		return -1;
	}

	if(length < 0)
	{
		printf("client[%d] exception exit\n", client_socket);
		g_pMysql->mysql_DeleteOnlineUsers(client_socket);
		return -2;
	}

	printf("client[%d] Nonblocking Read return : %d\n", client_socket, nRet);
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

int CreateHeader(Header* pHead, WU_uint16_t usCode,
	WU_uint32_t uiDataLen, WU_uint64_t ullDstId)
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
	int nRet = 0;

	Header hd;
	memset(&hd, 0, sizeof(Header));
	CreateHeader(&hd, KEEP_ALIVE_RSP, sizeof(KeepAliveRsp), ullClientID);

	Net_Send(client_socket, (char*)&hd, sizeof(hd), 0);

	KeepAliveRsp kar;
	kar.usAliveSeq = usAliveSeq;

	Net_Send(client_socket, (char*)&kar, sizeof(KeepAliveRsp), 0);

	return nRet;
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
	int nRet = 0;

	Header hd;
	memset(&hd, 0, sizeof(Header));
	CreateHeader(&hd, LOGIN_RSP, sizeof(LoginRsp), ullClientID);

	Net_Send(client_socket, (char*)&hd, sizeof(hd), 0);

	LoginRsp lr;
	memset(&lr, 0, sizeof(LoginRsp));
	lr.ucResult = ucResult;
	switch(ucResult)
	{
	case 0:
		memcpy(lr.szReason, "success", 128);
		break;
	case 1:
		memcpy(lr.szReason, "username or password error", 128);
		break;
	case 2:
		memcpy(lr.szReason, "user has login", 128);
		break;
	case 3:
		memcpy(lr.szReason, "unknown error", 128);
		break;
	}

	Net_Send(client_socket, (char*)&lr, sizeof(LoginRsp), 0);

	return nRet;
}

unsigned long long Login_Req_Function(int client_socket, Header* pHeader, const char* RecvBuffer)
{
	printf("LOGIN_REQ\n");

	LoginReq* pLoginReq = (LoginReq*)RecvBuffer;
	printf("username:%s, password:%s\n", pLoginReq->szUserName, pLoginReq->szPassWord);
	
	unsigned long long ullClientID = g_pMysql->mysql_GetUserID((char*)pLoginReq->szUserName, (char*)pLoginReq->szPassWord);
	
	unsigned char ucResult = 0;
	if (ullClientID > 0)
	{
		if(g_pMysql->mysql_AddOnlineUsers(client_socket, ullClientID) == 1)
		{
			printf("%s login success\n", (char*)pLoginReq->szUserName);
		}
		else
		{
			printf("%s login failed, user has login\n", (char*)pLoginReq->szUserName);
			ucResult = 2;
		}
	}
	else
	{
		printf("%s login failed, username or password error\n", (char*)pLoginReq->szUserName);
		ucResult = 1;
	}

	Login_Rsp_Function(client_socket, ullClientID, ucResult);

	return ullClientID;
}

bool UserIsOnline(unsigned long long ullClientID)
{
	printf("UserIsOnline\n");

	int nRet = g_pMysql->mysql_SelectOnlineUsers(ullClientID);
	if(nRet >= 1) return true;
	else return false;
}

int GetUserList_Rsp_Function(int client_socket, unsigned long long ullClientID)
{
	printf("GET_USER_LIST_RSP\n");

	int nRet = 0;

	Header hd;
	memset(&hd, 0, sizeof(Header));
	CreateHeader(&hd, LOGIN_RSP, sizeof(LoginRsp), ullClientID);

	Net_Send(client_socket, (char*)&hd, sizeof(hd), 0);

	vector<UserNet*> vecUserList;
	g_pMysql->mysql_SelectUserList(vecUserList);
	char* szUserList = g_MyJson.SetUserListJasonData(vecUserList);
	int nCount = vecUserList.size();
	for(int i = 0; i < nCount; i++)
	{
		if(vecUserList[i] != NULL)
		{
			delete vecUserList[i];
			vecUserList[i] = NULL;
		}
	}

	UserListRsp ulr;
	memset(&ulr, 0, sizeof(UserListRsp));
	ulr.usLen = strlen(szUserList);
	memcpy(&ulr+sizeof(UserListRsp), szUserList, ulr.usLen);

	Net_Send(client_socket, (char*)&ulr, sizeof(LoginRsp)+ulr.usLen, 0);

	cJason_free(szUserList);

	return nRet;
}

int GetUserList_Req_Function(int client_socket, Header* pHeader, unsigned long long ullClientID)
{
	if(!UserIsOnline(ullClientID)) return -1;

	printf("GET_USER_LIST_REQ\n");

	int nRet = 0;

	GetUserList_Rsp_Function(client_socket, ullClientID);

	return nRet;
}

#endif