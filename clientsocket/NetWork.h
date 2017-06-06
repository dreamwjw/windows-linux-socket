#pragma once

#include "protocol.h"

class CNetWork
{
public:
	CNetWork(void);
	~CNetWork(void);

public:
	int NonblockingRead(int ifd, unsigned int uiTimeOut);
	int NonblockingWrite(int ifd, unsigned int uiTimeOut);
	int Net_Receive(int client_socket, char* buf, int len, int flag);
	int Net_Send(int client_socket, char* buf, int len, int flag);
	bool SearchPacketHeader(int fd, char *buf,const int &iBufLen);
	int CreateHeader(Header* pHead, WU_uint16_t usCode, WU_uint32_t uiDataLen, WU_uint64_t ullDstId);
	int Keep_Alive_Req_Function(int sclient);
	int Keep_Alive_Req_Function_CP(int sclient);
	int Keep_Alive_Rsp_Function(const char* RecvBuffer);
	int Login_Req_Function(int sclient);
	int Login_Req_Function_CP(int sclient);
	int Login_Rsp_Function(int sclient, const char* RecvBuffer);
	int GetUserList_Req_Function(int sclient);
	int GetUserList_Req_Function_CP(int sclient);
	int GetUserList_Rsp_Function(int sclient, const char* RecvBuffer);
	int TalkWithUser_Req_Function(int sclient);
	int TalkWithUser_Req_Function_CP(int sclient);
	int TalkWithUser_Rsp_Function(int sclient, const char* RecvBuffer);
	int mystrchr(char* str, int c, char* szUserName, char* szContent);
};

