
// ClientSocketDlg.h : 头文件
//

#pragma once
#include "afxcmn.h"

#include "./socketclass/ClientSocketSQL.h"

// CClientSocketDlg 对话框
class CClientSocketDlg : public CDialogEx
{
// 构造
public:
	CClientSocketDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_CLIENTSOCKET_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	// the listctrl of student from database table
	CListCtrl m_lcStudent;
	CClientSocketSQL m_clientsocketsql;
	afx_msg void OnBnClickedBtnselect();
	afx_msg void OnBnClickedBtninsert();
	afx_msg void OnBnClickedBtnupdate();
	afx_msg void OnBnClickedBtndelete();
public:
	void NoSelect();
};
