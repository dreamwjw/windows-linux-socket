
// ClientSocket.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CClientSocketApp:
// �йش����ʵ�֣������ ClientSocket.cpp
//

class CClientSocketApp : public CWinApp
{
public:
	CClientSocketApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CClientSocketApp theApp;