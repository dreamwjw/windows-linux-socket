#include <STDIO.H>
#include <process.h>
#include <WINSOCK2.H> 

#include "MyJson.h"
#include "NetWork.h"
#include "Common.h"

CNetWork g_NetWork;

extern CCommon g_Common;
extern WU_uint64_t g_ullMacID;
extern CMyJson g_MyJson;
extern unsigned short g_usAliveSeq;
extern char g_szUserName[32];

CNetWork::CNetWork(void)
{
}


CNetWork::~CNetWork(void)
{
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
		g_Common.MyOutputDebugString(" Nonblocking Read error, return[%d]\n", nRet);
	}

	int length = recv(client_socket, buf, len, flag);

	if(length == SOCKET_ERROR) 
	{
		g_Common.MyOutputDebugString("recv error\n");
		return -1;
	}

	g_Common.MyOutputDebugString("Nonblocking Read return : %d\n", nRet);
	g_Common.MyOutputDebugString("recv length : %d\n", length);

	return length;
}

int CNetWork::Net_Send(int client_socket, char* buf, int len, int flag)
{
	int nRet = NonblockingWrite(client_socket, WU_NETWORK_TIMEOUT);

	if(nRet == 0 || nRet == SOCKET_ERROR)
	{
		g_Common.MyOutputDebugString("Nonblocking Write error, return[%d]\n", nRet);
	}

	int length = send(client_socket, buf, len, flag);

	if(length == SOCKET_ERROR) 
	{
		g_Common.MyOutputDebugString("send error\n");
		return -1;
	}

	g_Common.MyOutputDebugString("Nonblocking Write return : %d\n", nRet);
	g_Common.MyOutputDebugString("send length : %d\n", length);

	return length;
}

bool CNetWork::SearchPacketHeader(int fd, char *buf,const int &iBufLen)
{
	if(iBufLen < WU_HEADER_FLAG_LEN)
	{
		g_Common.MyOutputDebugString("Search PacketHeader error iBufflen [%d] not enough\n",iBufLen);
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
			g_Common.MyOutputDebugString("Search PacketHeader error iRes :%d\n",iRes);
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
		g_Common.MyOutputDebugString("ok find Packet Header[%s],iRevByes[%d]\n",ucPacketHeader,iRevBytes);
		memcpy(buf,ucPacketHeader,WU_HEADER_FLAG_LEN);
		return true;
	}
	else
	{
		g_Common.MyOutputDebugString(" cannot Search PacketHeader[%s],iRevBytes[%d]\n",ucPacketHeader,iRevBytes);
		return false;
	}
}

int CNetWork::CreateHeader(Header* pHead, WU_uint16_t usCode, WU_uint32_t uiDataLen, WU_uint64_t ullDstId)
{
	memset(pHead, 0, sizeof(Header));
	memcpy(pHead->szFlag, WU_HEADER_FLAG, WU_HEADER_FLAG_LEN);
	pHead->usCode = usCode;
	pHead->uiTotalLength = uiDataLen + WU_HEADER_LEN;
	pHead->ullSrcId = g_ullMacID;
	pHead->ullDstId = ullDstId;

	return 0;
}

int CNetWork::Keep_Alive_Req_Function(int sclient)
{
	Header hd;
	memset(&hd, 0, sizeof(Header));
	CreateHeader(&hd, KEEP_ALIVE_REQ, sizeof(KeepAliveReq), WU_SERVER_ID);

	Net_Send(sclient, (char*)&hd, sizeof(hd), 0);

	KeepAliveReq kar;
	memset(&kar, 0, sizeof(KeepAliveReq));
	kar.usAliveSeq = g_usAliveSeq;

	Net_Send(sclient, (char*)&kar, sizeof(KeepAliveReq), 0);

	if(g_usAliveSeq > 10000)
		g_usAliveSeq = 0;
	g_usAliveSeq++;

	return 0;
}

int CNetWork::Keep_Alive_Req_Function_CP(int sclient)
{
	char* szData = (char*)malloc(sizeof(Header) + sizeof(KeepAliveReq));
	Header* pHeader = (Header*)szData;
	CreateHeader(pHeader, KEEP_ALIVE_REQ, sizeof(KeepAliveReq), WU_SERVER_ID);
	KeepAliveReq* pKeepAliveReq = (KeepAliveReq*)(szData+sizeof(Header));
	pKeepAliveReq->usAliveSeq = g_usAliveSeq;
	Net_Send(sclient, szData, sizeof(Header) + sizeof(KeepAliveReq), 0);
	free(szData);

	if(g_usAliveSeq > 10000)
		g_usAliveSeq = 0;
	g_usAliveSeq++;

	return 0;
}

int CNetWork::Keep_Alive_Rsp_Function(const char* RecvBuffer)
{
	g_Common.MyOutputDebugString("KEEP_ALIVE_RSP\n");

	KeepAliveRsp* pKeepAliveRsp = (KeepAliveRsp*)RecvBuffer;
	g_Common.MyOutputDebugString("alive seq : [%u]\n", pKeepAliveRsp->usAliveSeq);

	return 0;
}

int CNetWork::Login_Req_Function(int sclient)
{
	char szPassWord[32] = {0};
	printf("UserName:");
	scanf_s("%s", g_szUserName, 32);
	//getchar();
	printf("PassWord:");
	scanf_s("%s", szPassWord, 32);

	Header hd;
	CreateHeader(&hd, LOGIN_REQ, sizeof(LoginReq), WU_SERVER_ID);

	Net_Send(sclient, (char*)&hd, sizeof(hd), 0);

	LoginReq lr;
	memset(&lr, 0, sizeof(LoginReq));
	memcpy(lr.szUserName, g_szUserName, 32);
	memcpy(lr.szPassWord, szPassWord, 32);

	Net_Send(sclient, (char*)&lr, sizeof(LoginReq), 0);

	return 0;
}

int CNetWork::Login_Req_Function_CP(int sclient)
{
	char szPassWord[32] = {0};
	printf("UserName:");
	scanf_s("%s", g_szUserName, 32);
	//getchar();
	printf("PassWord:");
	scanf_s("%s", szPassWord, 32);

	char* szData = (char*)malloc(sizeof(Header) + sizeof(LoginReq));
	Header* pHeader = (Header*)szData;
	CreateHeader(pHeader, LOGIN_REQ, sizeof(LoginReq), WU_SERVER_ID);
	LoginReq* pLoginReq = (LoginReq*)(szData+sizeof(Header));
	memcpy(pLoginReq->szUserName, g_szUserName, 32);
	memcpy(pLoginReq->szPassWord, szPassWord, 32);
	Net_Send(sclient, szData, sizeof(Header) + sizeof(LoginReq), 0);
	free(szData);

	return 0;
}

int CNetWork::Login_Rsp_Function(int sclient, const char* RecvBuffer)
{
	printf("LOGIN_RSP\n");

	LoginRsp* pLoginRsp = (LoginRsp*)RecvBuffer;
	if(pLoginRsp->ucResult == 0)
	{
		printf("login success, reason:%s\n", pLoginRsp->szReason);
	}
	else
	{
		printf("login failed, reason:%s\n", pLoginRsp->szReason);
	}

	return 0;
}

int CNetWork::GetUserList_Req_Function(int sclient)
{
	Header hd;
	CreateHeader(&hd, GET_USER_LIST_REQ, 0, WU_SERVER_ID);

	Net_Send(sclient, (char*)&hd, sizeof(hd), 0);

	return 0;
}

int CNetWork::GetUserList_Req_Function_CP(int sclient)
{
	Header hd;
	CreateHeader(&hd, GET_USER_LIST_REQ, 0, WU_SERVER_ID);

	Net_Send(sclient, (char*)&hd, sizeof(hd), 0);

	return 0;
}

int CNetWork::GetUserList_Rsp_Function(int sclient, const char* RecvBuffer)
{
	printf("GET_USER_LIST_RSP\n");

	UserListRsp* pUserListRsp = (UserListRsp*)RecvBuffer;
	char* szUserList = new char[pUserListRsp->usLen];
	memcpy(szUserList, RecvBuffer+sizeof(UserListRsp), pUserListRsp->usLen);

	vector<UserNet*> vecUserList;
	g_MyJson.GetUserListJasonData(szUserList, vecUserList);
	delete[] szUserList;

	int nCount = vecUserList.size();
	for(int i = 0; i < nCount; i++)
	{
		printf("UserName: %s\t", vecUserList[i]->szUserName);
		printf("Online: %d\n", vecUserList[i]->bIsOnline?1:0);
		if(vecUserList[i] != NULL)
		{
			delete vecUserList[i];
			vecUserList[i] = NULL;
		}
	}

	return 0;
}

int CNetWork::TalkWithUser_Req_Function(int sclient)
{
	printf("please follow this format, \"username,......\" \n");
	char szText[1024] = {0};
	scanf_s("%s", szText, 1024);

	char szContent[1024] = {0};
	char szUserName[32] = {0};
	g_Common.mystrchr(szText, ',', szUserName, szContent);

	Header hd;
	memset(&hd, 0, sizeof(Header));
	CreateHeader(&hd, TALK_WITH_USER_REQ, strlen(szContent)+sizeof(TalkWithUser), WU_SERVER_ID);

	Net_Send(sclient, (char*)&hd, sizeof(hd), 0);

	TalkWithUser twu;
	memset(&twu, 0, sizeof(TalkWithUser));
	twu.usLen = strlen(szContent);
	memcpy(twu.szFromUser, g_szUserName, 32);
	memcpy(twu.szToUser, szUserName, 32);
	char* szData = (char*)malloc(twu.usLen + sizeof(TalkWithUser));
	memcpy(szData, &twu, sizeof(TalkWithUser));
	memcpy(szData+sizeof(TalkWithUser), szContent, twu.usLen);

	Net_Send(sclient, szData, sizeof(TalkWithUser)+twu.usLen, 0);

	free(szData);

	return 0;
}

int CNetWork::TalkWithUser_Req_Function_CP(int sclient)
{
	printf("please follow this format, \"username,......\" \n");
	char szText[1024] = {0};
	scanf_s("%s", szText, 1024);

	char szContent[1024] = {0};
	char szUserName[32] = {0};
	g_Common.mystrchr(szText, ',', szUserName, szContent);

	char* szData = (char*)malloc(sizeof(Header) + sizeof(TalkWithUser)+strlen(szContent));
	Header* pHeader = (Header*)szData;
	CreateHeader(pHeader, TALK_WITH_USER_REQ, strlen(szContent)+sizeof(TalkWithUser), WU_SERVER_ID);
	TalkWithUser* pTalkWithUser = (TalkWithUser*)(szData+sizeof(Header));
	memcpy(pTalkWithUser->szFromUser, g_szUserName, 32);
	memcpy(pTalkWithUser->szToUser, szUserName, 32);
	pTalkWithUser->usLen = strlen(szContent);
	//这句代码错误，因为pTalkWithUser是TalkWithUser类型的指针，如果内存拷贝超出TalkWithUser的长度则拷贝失败
	//memcpy(pTalkWithUser+sizeof(TalkWithUser), szContent, strlen(szContent));
	memcpy(szData+sizeof(Header)+sizeof(TalkWithUser), szContent, strlen(szContent));
	Net_Send(sclient, szData, sizeof(Header)+sizeof(TalkWithUser)+strlen(szContent), 0);
	free(szData);

	return 0;
}

int CNetWork::TalkWithUser_Rsp_Function(int sclient, const char* RecvBuffer)
{
	TalkWithUser* pTalkWithUser = (TalkWithUser*)RecvBuffer;
	char* szContent = (char*)malloc(pTalkWithUser->usLen + 1);
	memset(szContent, 0, pTalkWithUser->usLen + 1);
	//szContent[pTalkWithUser->usLen + 1] = '\0';
	memcpy(szContent, RecvBuffer+sizeof(TalkWithUser), pTalkWithUser->usLen);

	printf("%s say to %s : %s \n", pTalkWithUser->szFromUser, pTalkWithUser->szToUser, szContent);

	free(szContent);

	return 0;
}

