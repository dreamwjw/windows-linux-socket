#pragma once
class CCommon
{
public:
	CCommon(void);
	~CCommon(void);

public:
	int mystrchr(char* str, int c, char* szUserName, char* szContent);
	void MyOutputDebugString(const char *sFormatString, ...);
};

