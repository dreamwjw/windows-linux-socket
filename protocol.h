#ifndef PROTOCOL_H
#define PROTOCOL_H

#pragma pack(push)
#pragma pack (1)

typedef unsigned char          uint8_t;
typedef unsigned short         uint16_t;
typedef unsigned int           uint32_t;
typedef unsigned long long     uint64_t;
typedef char                   int8_t;
typedef short                  int16_t;
typedef int                    int32_t;
typedef long long              int64_t;

#define LOGIN_REQ 0x2001                   // Client -> Server  --- struct LoginReq
#define LOGIN_RSP (LOGIN_REQ + 1)          // Server -> Client  --- struct LoginRsp

typedef struct tagHeader
{
	uint8_t szFlag[4];                     //default "WULI"
	uint16_t usCode;                       //Request code
	uint32_t uiTotalLength;                //Total packet length(header+data)
	uint64_t ullSrcId;                     //source ID
	uint64_t ullDstId;                     //destination ID
}Header;

typedef struct tagLoginReq
{
	uint8_t szUserName[32];               //user name 
	uint8_t szPassWord[32];               //password
}LoginReq;

typedef struct tagLoginRsp
{
	uint8_t ucResult;                     //0-success£¬1-user name error£¬2-password error
	uint8_t szReason[128];
}LoginRsp;

#pragma pack(pop)

#endif