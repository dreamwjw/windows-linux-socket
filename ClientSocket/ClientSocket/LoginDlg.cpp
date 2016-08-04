// LoginDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "ClientSocket.h"
#include "LoginDlg.h"
#include "afxdialogex.h"

// CLoginDlg 对话框

IMPLEMENT_DYNAMIC(CLoginDlg, CDialogEx)

CLoginDlg::CLoginDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CLoginDlg::IDD, pParent)
	, m_strName(_T(""))
	, m_strPassword(_T(""))
{

}

CLoginDlg::~CLoginDlg()
{
}

void CLoginDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_NAME, m_strName);
	DDX_Text(pDX, IDC_EDIT_PASSWORD, m_strPassword);
}


BEGIN_MESSAGE_MAP(CLoginDlg, CDialogEx)
	ON_BN_CLICKED(IDOK, &CLoginDlg::OnBnClickedOk)
	ON_WM_CREATE()
END_MESSAGE_MAP()


// CLoginDlg 消息处理程序


void CLoginDlg::OnBnClickedOk()
{
	UpdateData(true);

	char strSQL[CClientSocketSQL::BUFFER_SIZE + 1] = {0};
	sprintf_s(strSQL, CClientSocketSQL::BUFFER_SIZE, "select * from user where name = '%s' and password = %s",
		m_strName.GetString(), m_strPassword.GetString());

	vector<vector<string>> vvResult;
	m_clientsocketsql.Select(strSQL, vvResult);

	if(vvResult[0].size() == 4)
	{
		CDialogEx::OnOK();
	}
	else
	{
		MessageBox("登录失败，帐号或者密码错误!");
	}
}


int CLoginDlg::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDialogEx::OnCreate(lpCreateStruct) == -1)
		return -1;

	if(!m_clientsocketsql.Init()) 
	{
		MessageBox("Socket Init Failed!");
		SendMessage(WM_CLOSE);
	}
	if(!m_clientsocketsql.Connect("192.168.241.128", 4000))
	{
		MessageBox("Socket Connect Failed!");
		SendMessage(WM_CLOSE);
	}

	return 0;
}
