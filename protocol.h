#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdio.h>
#include <stdlib.h>

#pragma pack(push)
#pragma pack (1)

typedef unsigned char          WU_uint8_t;
typedef unsigned short         WU_uint16_t;
typedef unsigned int           WU_uint32_t;
typedef unsigned long long     WU_uint64_t;
typedef char                   WU_int8_t;
typedef short                  WU_int16_t;
typedef int                    WU_int32_t;
typedef long long              WU_int64_t;

#define LOGIN_REQ 0x2001                   // Client -> Server  --- struct LoginReq
#define LOGIN_RSP (LOGIN_REQ + 1)          // Server -> Client  --- struct LoginRsp

typedef struct tagHeader
{
	WU_uint8_t szFlag[4];                     //default "WULI"
	WU_uint16_t usCode;                       //Request code
	WU_uint32_t uiTotalLength;                //Total packet length(header+data)
	WU_uint64_t ullSrcId;                     //source ID
	WU_uint64_t ullDstId;                     //destination ID
}Header;

typedef struct tagLoginReq
{
	WU_uint8_t szUserName[32];               //user name 
	WU_uint8_t szPassWord[32];               //password
}LoginReq;

typedef struct tagLoginRsp
{
	WU_uint8_t ucResult;                     //0-success£¬1-user name error£¬2-password error
	WU_uint8_t szReason[128];
}LoginRsp;

#pragma pack(pop)

#endif