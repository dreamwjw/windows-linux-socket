#include <WINSOCK2.H>  
#include <STDIO.H>
#include <process.h>//_beginthreadex
#include <Iphlpapi.h>

#include "protocol.h"
#include "MyJson.h"
#include "NetWork.h"
#include "Common.h"

#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Iphlpapi.lib")

WU_uint64_t GetLocalMac();//<Iphlpapi.h>
static PIP_ADAPTER_INFO g_pInfo = NULL;//<Iphlpapi.h>
WU_uint64_t g_ullMacID = GetLocalMac();

unsigned short g_usAliveSeq = 0;
char g_szUserName[32] = {0};

extern CNetWork g_NetWork;
extern CCommon g_Common;

int testsend(int sclient);
void MyOutputDebugString(const char *sFormatString, ...);

unsigned int __stdcall threadRecv(void* pParam);
unsigned int __stdcall threadSendHeart(void* pParam);

void showmenu();
void operation(int sclient);
void stop(int sclient);

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
	g_Common.MyOutputDebugString("threadSendHeart begin\n");

	int sclient = (int)(*((int*)pParam));
	long long llLastTime = time(NULL);

	while(!g_bExit)
	{
		long long llTimeNow = time(NULL);
		if(llTimeNow - llLastTime > 3)
		{
			g_NetWork.Keep_Alive_Req_Function_CP(sclient);
			llLastTime = llTimeNow;
		}

		Sleep(1000);
	}

	g_Common.MyOutputDebugString("threadSendHeart end\n");

	return 0;
}

unsigned int __stdcall threadRecv(void* pParam)
{
	g_Common.MyOutputDebugString("threadRecv begin\n");

	int sclient = (int)(*((int*)pParam));
	int length = 0;

	while(!g_bExit)
	{
		Header hd;
		memset(&hd, 0, sizeof(Header));
		length = g_NetWork.Net_Receive(sclient, (char*)&hd, sizeof(Header), 0);
		if(length < 0) break; 
		else if(length == 0)
		{
			g_NetWork.Keep_Alive_Req_Function(sclient);
			continue;
		}

		if(memcmp(hd.szFlag, WU_HEADER_FLAG, WU_HEADER_FLAG_LEN) != 0)
		{
			g_Common.MyOutputDebugString("invalid header flag; [%0x],[%0x],[%0x],[%0x]\n", hd.szFlag[0], hd.szFlag[1],
				hd.szFlag[2], hd.szFlag[3]);

			bool bFlag = false;
			if(g_NetWork.SearchPacketHeader(sclient, (char *)hd.szFlag, WU_HEADER_FLAG_LEN))
			{
				length = g_NetWork.Net_Receive(sclient, (char*)&hd.usCode, sizeof(Header) - WU_HEADER_FLAG_LEN, 0); 
				if(length == sizeof(Header) - WU_HEADER_FLAG_LEN)
					bFlag = true;
			}
			if(!bFlag) break;
		}

		unsigned int uiDataLen = hd.uiTotalLength - WU_HEADER_LEN;
		char *pDataBuff = (char *)malloc(uiDataLen);
		if(pDataBuff == NULL)
		{
			g_Common.MyOutputDebugString("Insufficient memory available\n");
			break;
		}
		memset(pDataBuff, 0, uiDataLen);

		length = g_NetWork.Net_Receive(sclient, pDataBuff, uiDataLen, 0);
		if (length != uiDataLen)
		{
			g_Common.MyOutputDebugString("net receive data error, receive size[%d], should receive[%u], cmd is [0x%x]\n",
				length, uiDataLen, hd.usCode);
			free(pDataBuff);
			pDataBuff = NULL;
			break;
		}

		switch(hd.usCode)
		{
		case KEEP_ALIVE_RSP:
			{
				g_NetWork.Keep_Alive_Rsp_Function(pDataBuff);
			}
			break;
		case LOGIN_RSP:
			{
				g_NetWork.Login_Rsp_Function(sclient, pDataBuff);
			}
			break;
		case GET_USER_LIST_RSP:
			{
				g_NetWork.GetUserList_Rsp_Function(sclient, pDataBuff);
			}
			break;
		case TALK_WITH_USER_RSP:
			{
				g_NetWork.TalkWithUser_Rsp_Function(sclient, pDataBuff);
			}
			break;
		default:
			break;
		}

		free(pDataBuff);
		pDataBuff = NULL;

		//Sleep(10);
	}

	g_Common.MyOutputDebugString("threadRecv end\n");

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

				//清屏之后清空输入
				while(getchar() != '\n') ;
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
				g_NetWork.Login_Req_Function_CP(sclient);
			}
			break;
		case 3:
			{
				g_NetWork.GetUserList_Req_Function_CP(sclient);
			}
			break;
		case 4:
			{
				g_NetWork.TalkWithUser_Req_Function_CP(sclient);
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