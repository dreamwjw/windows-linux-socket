//#include "stdafx.h"  
#include <WINSOCK2.H>  
#include <STDIO.H>
#include <Iphlpapi.h>
#include <process.h>

#include "protocol.h"
#include "MyJson.h"

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Iphlpapi.lib")

WU_uint64_t GetLocalMac();
static PIP_ADAPTER_INFO g_pInfo=NULL;
static WU_uint64_t g_ullMacID = GetLocalMac();
static unsigned short g_usAliveSeq = 0;
static CMyJson g_MyJson;
static char g_szUserName[32] = {0};

int testsend(int sclient);
void MyOutputDebugString(const char *sFormatString, ...);

unsigned int __stdcall threadRecv(void* pParam);
unsigned int __stdcall threadSendHeart(void* pParam);

void showmenu();
void operation(int sclient);
void stop(int sclient);

int Net_Receive(int client_socket, char* buf, int len, int flag);
int Net_Send(int client_socket, char* buf, int len, int flag);
int NonblockingWrite(int ifd, unsigned int uiTimeOut);
int NonblockingRead(int ifd, unsigned int uiTimeOut);
bool SearchPacketHeader(int fd, char *buf,const int &iBufLen);

int CreateHeader(Header* pHead, WU_uint16_t usCode, WU_uint32_t uiDataLen, WU_uint64_t ullDstId);

int Keep_Alive_Req_Function(int sclient);
int Keep_Alive_Rsp_Function(const char* RecvBuffer);
int Login_Req_Function(int sclient);
int Login_Rsp_Function(int sclient, const char* RecvBuffer);
int GetUserList_Req_Function(int sclient);
int GetUserList_Rsp_Function(int sclient, const char* RecvBuffer);
int TalkWithUser_Req_Function(int sclient);
int TalkWithUser_Rsp_Function(int sclient, const char* RecvBuffer);

int mystrchr(char* str, int c, char* szUserName, char* szContent);

bool g_bExit = false;
HANDLE g_hThreadRecv = NULL;
HANDLE g_hThreadSendHeart = NULL;

int main(int argc, char* argv[])  
{  
	WORD sockVersion = MAKEWORD(2,2);  
	WSADATA data;   
	if(WSAStartup(sockVersion, &data) != 0)  
	{  
		return 0;  
	}

	int sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  
	if(sclient == INVALID_SOCKET)  
	{  
		printf("invalid socket !\n");  
		return 0;  
	}  

	sockaddr_in serAddr;  
	serAddr.sin_family = AF_INET;  
	serAddr.sin_port = htons(7878);
	serAddr.sin_addr.S_un.S_addr = inet_addr("192.168.0.88");   
	if (connect(sclient, (sockaddr *)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)  
	{  
		printf("connect error !\n");  
		closesocket(sclient);  
		return 0;  
	}

	g_hThreadRecv = (HANDLE)_beginthreadex(NULL, 0, &threadRecv, (void*)&sclient, 0, NULL);
	g_hThreadSendHeart = (HANDLE)_beginthreadex(NULL, 0, &threadSendHeart, (void*)&sclient, 0, NULL);

	showmenu();
	operation(sclient);

	WSACleanup();

	return 0;  
}

void MyOutputDebugString(const char *sFormatString, ...)
{
	va_list va;
	HRESULT hr = S_OK;
	const DWORD MYDEBUG_STRING_LEN = 1024;//一条日志的最大长度
	char message[MYDEBUG_STRING_LEN] = {0};

	va_start(va, sFormatString);
	hr = _vsnprintf_s(message, MYDEBUG_STRING_LEN, sFormatString, va);
	va_end(va);

	if (SUCCEEDED(hr))
	{
		OutputDebugString(message);
	}
	else 
	{
		OutputDebugString("_vsnprintf_s failed");
	}
}

int testsend(int sclient)
{
	char sendData[20] = "Hello Server";  
	send(sclient, sendData, strlen(sendData), 0);  

	char recData[255] = {0};  
	int ret = recv(sclient, recData, 255, 0);
	if(ret > 0)  
	{  
		recData[ret] = 0x00;  
		printf(recData);  
	}

	return 0;
}

WU_uint64_t GetLocalMac()
{
	ULONG ulSize=0;
	int temp=0;
	temp = GetAdaptersInfo(g_pInfo,&ulSize);//第一处调用，获取缓冲区大小
	g_pInfo=(PIP_ADAPTER_INFO)malloc(ulSize);
	temp = GetAdaptersInfo(g_pInfo,&ulSize);

	if(temp == NO_ERROR)
		return reinterpret_cast<WU_uint64_t>(g_pInfo->Address);
	else return -1;
}

unsigned int __stdcall threadSendHeart(void* pParam)
{
	MyOutputDebugString("threadSendHeart begin\n");

	int sclient = (int)(*((int*)pParam));
	long long llLastTime = time(NULL);

	while(!g_bExit)
	{
		long long llTimeNow = time(NULL);
		if(llTimeNow - llLastTime > 3)
		{
			Keep_Alive_Req_Function(sclient);
			llLastTime = llTimeNow;
		}

		Sleep(1000);
	}

	MyOutputDebugString("threadSendHeart end\n");

	return 0;
}

unsigned int __stdcall threadRecv(void* pParam)
{
	MyOutputDebugString("threadRecv begin\n");

	int sclient = (int)(*((int*)pParam));
	int length = 0;

	while(!g_bExit)
	{
		Header hd;
		memset(&hd, 0, sizeof(Header));
		length = Net_Receive(sclient, (char*)&hd, sizeof(Header), 0);
		if(length < 0) break; 
		else if(length == 0)
		{
			Keep_Alive_Req_Function(sclient);
			continue;
		}

		if(memcmp(hd.szFlag, WU_HEADER_FLAG, WU_HEADER_FLAG_LEN) != 0)
		{
			MyOutputDebugString("invalid header flag; [%0x],[%0x],[%0x],[%0x]\n", hd.szFlag[0], hd.szFlag[1],
				hd.szFlag[2], hd.szFlag[3]);

			bool bFlag = false;
			if(SearchPacketHeader(sclient, (char *)hd.szFlag, WU_HEADER_FLAG_LEN))
			{
				length = Net_Receive(sclient, (char*)&hd.usCode, sizeof(Header) - WU_HEADER_FLAG_LEN, 0); 
				if(length == sizeof(Header) - WU_HEADER_FLAG_LEN)
					bFlag = true;
			}
			if(!bFlag) break;
		}

		unsigned int uiDataLen = hd.uiTotalLength - WU_HEADER_LEN;
		char *pDataBuff = (char *)malloc(uiDataLen);
		if(pDataBuff == NULL)
		{
			MyOutputDebugString("Insufficient memory available\n");
			break;
		}
		memset(pDataBuff, 0, uiDataLen);

		length = Net_Receive(sclient, pDataBuff, uiDataLen, 0);
		if (length != uiDataLen)
		{
			MyOutputDebugString("net receive data error, receive size[%d], should receive[%u], cmd is [0x%x]\n",
				length, uiDataLen, hd.usCode);
			free(pDataBuff);
			pDataBuff = NULL;
			break;
		}

		switch(hd.usCode)
		{
		case KEEP_ALIVE_RSP:
			{
				Keep_Alive_Rsp_Function(pDataBuff);
			}
			break;
		case LOGIN_RSP:
			{
				Login_Rsp_Function(sclient, pDataBuff);
			}
			break;
		case GET_USER_LIST_RSP:
			{
				GetUserList_Rsp_Function(sclient, pDataBuff);
			}
			break;
		case TALK_WITH_USER_RSP:
			{
				TalkWithUser_Rsp_Function(sclient, pDataBuff);
			}
			break;
		default:
			break;
		}

		free(pDataBuff);
		pDataBuff = NULL;

		//Sleep(10);
	}

	MyOutputDebugString("threadRecv end\n");

	return 0;
}

void operation(int sclient)
{
	while(1)
	{
		int nScan = 0;
		scanf_s("%d", &nScan);

		switch(nScan)
		{
		case 0:
			{
				system("cls");
				showmenu();
			}
			break;
		case 1:
			{
				stop(sclient);
				return;	
			}
			break;
		case 2:
			{
				Login_Req_Function(sclient);
			}
			break;
		case 3:
			{
				GetUserList_Req_Function(sclient);
			}
			break;
		case 4:
			{
				TalkWithUser_Req_Function(sclient);
			}
			break;
		default:
			break;
		}
	}
}

void showmenu()
{
	printf("operation menu:\n");
	printf("0     ------     clear\n");
	printf("1     ------     exit\n");
	printf("2     ------     login\n");
	printf("3     ------     get user list\n");
	printf("4     ------     talk with user\n");
}

void stop(int sclient)
{
	g_bExit = true;

	closesocket(sclient);  

	WaitForSingleObject(g_hThreadRecv, INFINITE);
	CloseHandle(g_hThreadRecv);
	g_hThreadRecv = NULL;

	WaitForSingleObject(g_hThreadSendHeart, INFINITE);
	CloseHandle(g_hThreadSendHeart);
	g_hThreadSendHeart = NULL;
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
		MyOutputDebugString(" Nonblocking Read error, return[%d]\n", nRet);
	}

	int length = recv(client_socket, buf, len, flag);

	if(length == SOCKET_ERROR) 
	{
		MyOutputDebugString("recv error\n");
		return -1;
	}

	MyOutputDebugString("Nonblocking Read return : %d\n", nRet);
	MyOutputDebugString("recv length : %d\n", length);

	return length;
}

int Net_Send(int client_socket, char* buf, int len, int flag)
{
	int nRet = NonblockingWrite(client_socket, WU_NETWORK_TIMEOUT);

	if(nRet == 0 || nRet == SOCKET_ERROR)
	{
		MyOutputDebugString("Nonblocking Write error, return[%d]\n", nRet);
	}

	int length = send(client_socket, buf, len, flag);

	if(length == SOCKET_ERROR) 
	{
		MyOutputDebugString("send error\n");
		return -1;
	}

	MyOutputDebugString("Nonblocking Write return : %d\n", nRet);
	MyOutputDebugString("send length : %d\n", length);

	return length;
}

bool SearchPacketHeader(int fd, char *buf,const int &iBufLen)
{
	if(iBufLen < WU_HEADER_FLAG_LEN)
	{
		MyOutputDebugString("Search PacketHeader error iBufflen [%d] not enough\n",iBufLen);
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
			MyOutputDebugString("Search PacketHeader error iRes :%d\n",iRes);
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
		MyOutputDebugString("ok find Packet Header[%s],iRevByes[%d]\n",ucPacketHeader,iRevBytes);
		memcpy(buf,ucPacketHeader,WU_HEADER_FLAG_LEN);
		return true;
	}
	else
	{
		MyOutputDebugString(" cannot Search PacketHeader[%s],iRevBytes[%d]\n",ucPacketHeader,iRevBytes);
		return false;
	}
}

int CreateHeader(Header* pHead, WU_uint16_t usCode, WU_uint32_t uiDataLen, WU_uint64_t ullDstId)
{
	memset(pHead, 0, sizeof(Header));
	memcpy(pHead->szFlag, WU_HEADER_FLAG, WU_HEADER_FLAG_LEN);
	pHead->usCode = usCode;
	pHead->uiTotalLength = uiDataLen + WU_HEADER_LEN;
	pHead->ullSrcId = g_ullMacID;
	pHead->ullDstId = ullDstId;

	return 0;
}

int Keep_Alive_Req_Function(int sclient)
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

int Keep_Alive_Rsp_Function(const char* RecvBuffer)
{
	MyOutputDebugString("KEEP_ALIVE_RSP\n");

	KeepAliveRsp* pKeepAliveRsp = (KeepAliveRsp*)RecvBuffer;
	MyOutputDebugString("alive seq : [%u]\n", pKeepAliveRsp->usAliveSeq);

	return 0;
}

int Login_Req_Function(int sclient)
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

int Login_Rsp_Function(int sclient, const char* RecvBuffer)
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

int GetUserList_Req_Function(int sclient)
{
	Header hd;
	CreateHeader(&hd, GET_USER_LIST_REQ, 0, WU_SERVER_ID);

	Net_Send(sclient, (char*)&hd, sizeof(hd), 0);

	return 0;
}

int GetUserList_Rsp_Function(int sclient, const char* RecvBuffer)
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

int TalkWithUser_Req_Function(int sclient)
{
	printf("please follow this format, \"username,......\" \n");
	char szText[1024] = {0};
	scanf_s("%s", szText, 1024);

	char szContent[1024] = {0};
	char szUserName[32] = {0};
	mystrchr(szText, ',', szUserName, szContent);

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

int TalkWithUser_Rsp_Function(int sclient, const char* RecvBuffer)
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

int mystrchr(char* str, int c, char* szUserName, char* szContent)
{
	char *pos = strchr(str,c);

	if(pos == NULL) return -1;

	memcpy(szUserName, str, pos - str);
	strcpy_s(szContent, 1024, pos+1);

	return 0;
}