#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdio.h>
#include <stdlib.h>

#pragma pack(push)
#pragma pack (1)

#define WU_HEADER_LEN          26
#define WU_HEADER_FLAG_LEN     4
#define WU_HEADER_FLAG         "SK01"
#define WU_NETWORK_TIMEOUT	   1000 * 10
#define WU_SERVER_ID           0
#define WU_KEEP_ALIVE_TIMEOUT  10

typedef unsigned char          WU_uint8_t;
typedef unsigned short         WU_uint16_t;
typedef unsigned int           WU_uint32_t;
typedef unsigned long long     WU_uint64_t;
typedef char                   WU_int8_t;
typedef short                  WU_int16_t;
typedef int                    WU_int32_t;
typedef long long              WU_int64_t;

#define KEEP_ALIVE_REQ   0x2001                    // Client -> Server  --- struct LoginReq
#define KEEP_ALIVE_RSP   (KEEP_ALIVE_REQ + 1)      //CMS Server -> DVR/CMS Client

#define LOGIN_REQ 0x2003                           // Client -> Server  --- struct LoginReq
#define LOGIN_RSP (LOGIN_REQ + 1)                  // Server -> Client  --- struct LoginRsp

typedef struct tagHeader
{
	WU_uint8_t szFlag[4];                     //Default "WULI"
	WU_uint16_t usCode;                       //Request code
	WU_uint32_t uiTotalLength;                //Total packet length(header+data)
	WU_uint64_t ullSrcId;                     //Source ID
	WU_uint64_t ullDstId;                     //Destination ID
}Header;

typedef struct tagKeepAliveReq
{
	//WU_uint8_t ucAliveTime;                   //Send once every few seconds
	//WU_uint8_t ucTimeOut;                     //Maximum timeout
	WU_uint16_t usAliveSeq;                   //0~
}KeepAliveReq;

typedef struct tagKeepAliveRsp
{
	WU_uint16_t usAliveSeq;                   
}KeepAliveRsp;

typedef struct tagLoginReq
{
	WU_uint8_t szUserName[32];               //user name 
	WU_uint8_t szPassWord[32];               //password
}LoginReq;

typedef struct tagLoginRsp
{
	WU_uint8_t ucResult;                     //0 - success; 1 - username or password error; 2 - unknown error
	WU_uint8_t szReason[128];
}LoginRsp;

#pragma pack(pop)

#endif