#include <stdio.h>
#include <windows.h>

#include "Common.h"

 CCommon g_Common;

CCommon::CCommon(void)
{
}

CCommon::~CCommon(void)
{
}

int CCommon::mystrchr(char* str, int c, char* szUserName, char* szContent)
{
	char *pos = strchr(str,c);

	if(pos == NULL) return -1;

	memcpy(szUserName, str, pos - str);
	strcpy_s(szContent, 1024, pos+1);

	return 0;
}

void CCommon::MyOutputDebugString(const char *sFormatString, ...)
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