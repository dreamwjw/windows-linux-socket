
// ClientSocketDlg.h : ͷ�ļ�
//

#pragma once
#include "afxcmn.h"

#include "./socketclass/ClientSocketSQL.h"

// CClientSocketDlg �Ի���
class CClientSocketDlg : public CDialogEx
{
// ����
public:
	CClientSocketDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_CLIENTSOCKET_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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
