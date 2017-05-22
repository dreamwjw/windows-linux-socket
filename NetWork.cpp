#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "NetWork.h"
#include "mysql.h"
#include "MyJson.h"

#define SOCKET_ERROR -1

extern CMysql* g_pMysql;
CMyJson g_MyJson;

CNetWork::CNetWork(void)
{
}

CNetWork::~CNetWork(void)
{
}

char CNetWork::tolower(char c) {
	if(c >= 'A' && c <= 'Z')
		c += 'a' - 'A';
	return c;
}

int CNetWork::Init()
{
	g_pMysql = CMysql::GetInstance();
	if(g_pMysql == NULL)
	{
		printf("mysql connect failed");
		return -1;
	}
	
	return 0;
}

bool CNetWork::SearchPacketHeader(const int fd, char *buf,const int &iBufLen)
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

int CNetWork::NonblockingRead(int ifd, unsigned int uiTimeOut)
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

int CNetWork::NonblockingWrite(int ifd, unsigned int uiTimeOut)
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

int CNetWork::Net_Receive(int client_socket, char* buf, int len, int flag)
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

int CNetWork::Net_Send(int client_socket, char* buf, int len, int flag)
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

int CNetWork::CreateHeader(Header* pHead, WU_uint16_t usCode,
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

int CNetWork::Keep_Alive_Rsp_Function(int client_socket, unsigned long long ullClientID, unsigned short usAliveSeq)
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

int CNetWork::Keep_Alive_Req_Function(int client_socket, Header* pHeader, const char* RecvBuffer)
{
	printf("KEEP_ALIVE_REQ\n");

	if(RecvBuffer == NULL)
	{
		printf("RecvBuffer is null");
		return -1;
	}

	KeepAliveReq* pKeepAliveReq = (KeepAliveReq*)RecvBuffer;
	printf("alive seq : [%u]\n", pKeepAliveReq->usAliveSeq);

	Keep_Alive_Rsp_Function(client_socket, pHeader->ullSrcId, pKeepAliveReq->usAliveSeq);

	return 0;
}

int CNetWork::Login_Rsp_Function(int client_socket, unsigned long long ullClientID, unsigned char ucResult)
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

unsigned long long CNetWork::Login_Req_Function(int client_socket, Header* pHeader, const char* RecvBuffer, unsigned long long ullMacID)
{
	printf("LOGIN_REQ\n");

	if(RecvBuffer == NULL)
	{
		printf("RecvBuffer is null");
		return 0;
	}

	LoginReq* pLoginReq = (LoginReq*)RecvBuffer;
	printf("username:%s, password:%s\n", pLoginReq->szUserName, pLoginReq->szPassWord);

	unsigned long long ullClientID = g_pMysql->mysql_GetUserID((char*)pLoginReq->szUserName, (char*)pLoginReq->szPassWord);

	unsigned char ucResult = 0;
	if (ullClientID > 0)
	{
		if(g_pMysql->mysql_AddOnlineUsers(client_socket, ullClientID, ullMacID) == 1)
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

bool CNetWork::UserIsOnline(unsigned long long ullClientID)
{
	printf("UserIsOnline\n");

	int nRet = g_pMysql->mysql_SelectOnlineUsers(ullClientID);
	if(nRet >= 1) return true;
	else return false;
}

int CNetWork::GetUserList_Rsp_Function(int client_socket, unsigned long long ullClientID)
{
	printf("GET_USER_LIST_RSP\n");

	vector<UserNet*> vecUserList;
	g_pMysql->mysql_SelectUserList(vecUserList);
	char* szUserList = g_MyJson.SetUserListJasonData(vecUserList);
	printf("UserList : %s \n", szUserList);
	int nCount = vecUserList.size();
	for(int i = 0; i < nCount; i++)
	{
		if(vecUserList[i] != NULL)
		{
			delete vecUserList[i];
			vecUserList[i] = NULL;
		}
	}

	Header hd;
	memset(&hd, 0, sizeof(Header));
	CreateHeader(&hd, GET_USER_LIST_RSP, strlen(szUserList) + sizeof(UserListRsp), ullClientID);

	Net_Send(client_socket, (char*)&hd, sizeof(hd), 0);

	UserListRsp ulr;
	memset(&ulr, 0, sizeof(UserListRsp));
	ulr.usLen = strlen(szUserList);
	char* szData = (char*)malloc(ulr.usLen + sizeof(UserListRsp));
	memcpy(szData, &ulr, sizeof(UserListRsp));
	memcpy(szData+sizeof(UserListRsp), szUserList, ulr.usLen);

	Net_Send(client_socket, szData, sizeof(UserListRsp)+ulr.usLen, 0);

	cJason_free(szUserList);
	free(szData);

	return 0;
}

int CNetWork::GetUserList_Req_Function(int client_socket, unsigned long long ullClientID)
{
	printf("GET_USER_LIST_REQ\n");

	if(!UserIsOnline(ullClientID))
	{
		printf("user[%llu] is offline\n", ullClientID);
		return -1;
	}

	GetUserList_Rsp_Function(client_socket, ullClientID);

	return 0;
}

int CNetWork::TalkWithUser_Rsp_Function(int ToUserSocketID, unsigned long long ullToUserID, const char* szContent,
	const char* szFromUser, const char* szToUser)
{
	Header hd;
	memset(&hd, 0, sizeof(Header));
	CreateHeader(&hd, TALK_WITH_USER_RSP, strlen(szContent)+sizeof(TalkWithUser), ullToUserID);

	Net_Send(ToUserSocketID, (char*)&hd, sizeof(hd), 0);

	TalkWithUser twu;
	memset(&twu, 0, sizeof(TalkWithUser));
	twu.usLen = strlen(szContent);
	memcpy(twu.szFromUser, szFromUser, 32);
	memcpy(twu.szToUser, szToUser, 32);
	char* szData = (char*)malloc(twu.usLen + sizeof(TalkWithUser));
	memcpy(szData, &twu, sizeof(TalkWithUser));
	memcpy(szData+sizeof(TalkWithUser), szContent, twu.usLen);

	Net_Send(ToUserSocketID, szData, sizeof(TalkWithUser)+twu.usLen, 0);

	free(szData);

	return 0;
}

int CNetWork::TalkWithUser_Req_Function(int client_socket, const char* RecvBuffer, unsigned long long ullClientID)
{
	printf("TALK_WITH_USER_REQ\n");

	if(!UserIsOnline(ullClientID))
	{
		printf("user[%llu] is offline\n", ullClientID);
		return -1;
	}

	TalkWithUser* pTalkWithUser = (TalkWithUser*)RecvBuffer;
	char* szContent = (char*)malloc(pTalkWithUser->usLen);
	memcpy(szContent, RecvBuffer+sizeof(TalkWithUser), pTalkWithUser->usLen);

	unsigned long long ullUserID = 0;
	int nSocketID = 0;
	g_pMysql->mysql_SelectUserIDAndSocketID((char*)pTalkWithUser->szToUser, ullUserID, nSocketID);

	if(ullUserID ==0 || nSocketID == 0)
	{
		printf("user[%s] is offline\n", (char*)pTalkWithUser->szToUser);
		return -2;
	}

	TalkWithUser_Rsp_Function(nSocketID, ullUserID, szContent, 
		(char*)pTalkWithUser->szFromUser, (char*)pTalkWithUser->szToUser);

	free(szContent);

	return 0;
}