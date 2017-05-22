#pragma once

#include "protocol.h"

class CNetWork
{
public:
	CNetWork(void);
	~CNetWork(void);

private:
	char tolower(char c);

public:
	int Init();
	bool SearchPacketHeader(const int fd, char *buf,const int &iBufLen);
	int NonblockingRead(int ifd, unsigned int uiTimeOut);
	int NonblockingWrite(int ifd, unsigned int uiTimeOut);
	int Net_Receive(int client_socket, char* buf, int len, int flag);
	int Net_Send(int client_socket, char* buf, int len, int flag);
	int CreateHeader(Header* pHead, WU_uint16_t usCode, WU_uint32_t uiDataLen, WU_uint64_t ullDstId);
	int Keep_Alive_Rsp_Function(int client_socket, unsigned long long ullClientID, unsigned short usAliveSeq);
	int Keep_Alive_Req_Function(int client_socket, Header* pHeader, const char* RecvBuffer);
	int Login_Rsp_Function(int client_socket, unsigned long long ullClientID, unsigned char ucResult);
	unsigned long long Login_Req_Function(int client_socket, Header* pHeader, const char* RecvBuffer,
		unsigned long long ullMacID);
	bool UserIsOnline(unsigned long long ullClientID);
	int GetUserList_Rsp_Function(int client_socket, unsigned long long ullClientID);
	int GetUserList_Req_Function(int client_socket, unsigned long long ullClientID);
	int TalkWithUser_Rsp_Function(int ToUserSocketID, unsigned long long ullToUserID, const char* szContent,
		const char* szFromUser, const char* szToUser);
	int TalkWithUser_Req_Function(int client_socket, const char* RecvBuffer, unsigned long long ullClientID);

};