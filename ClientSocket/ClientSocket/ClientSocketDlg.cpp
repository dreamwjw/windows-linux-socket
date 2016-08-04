
// ClientSocketDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "ClientSocket.h"
#include "ClientSocketDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CClientSocketDlg �Ի���




CClientSocketDlg::CClientSocketDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CClientSocketDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CClientSocketDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LISTSTUDENT, m_lcStudent);
}

BEGIN_MESSAGE_MAP(CClientSocketDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTNSELECT, &CClientSocketDlg::OnBnClickedBtnselect)
	ON_BN_CLICKED(IDC_BTNINSERT, &CClientSocketDlg::OnBnClickedBtninsert)
	ON_BN_CLICKED(IDC_BTNUPDATE, &CClientSocketDlg::OnBnClickedBtnupdate)
	ON_BN_CLICKED(IDC_BTNDELETE, &CClientSocketDlg::OnBnClickedBtndelete)
END_MESSAGE_MAP()


// CClientSocketDlg ��Ϣ�������

BOOL CClientSocketDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO: �ڴ���Ӷ���ĳ�ʼ������
	//reset listcrtl stype
	LONG lStyle;
	lStyle = GetWindowLong(m_lcStudent.m_hWnd, GWL_STYLE);//��ȡ��ǰ����style
	//lStyle &= ~LVS_TYPEMASK; //�����ʾ��ʽλ
	lStyle |= LVS_REPORT; //���ñ��style
	SetWindowLong(m_lcStudent.m_hWnd, GWL_STYLE, lStyle);//����style

	//test insert function
	//m_lcStudent.InsertColumn(0, L"column1", LVCFMT_LEFT, 150);
	//m_lcStudent.InsertColumn(1, L"column2", LVCFMT_LEFT, 150);

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

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CClientSocketDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else 
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CClientSocketDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CClientSocketDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CClientSocketDlg::OnBnClickedBtnselect()
{
	/* delete all rows and columns for add new rows and columns */
	m_lcStudent.DeleteAllItems();
	while(m_lcStudent.DeleteColumn(0));

	char strSQL[CClientSocketSQL::BUFFER_SIZE + 1] = {0};
	GetDlgItemText(IDC_EDITSQL, strSQL, CClientSocketSQL::BUFFER_SIZE);
	SetDlgItemText(IDC_EDITSQL, "");

	if(strlen(strSQL) == 0) return;

	vector<vector<string>> vvResult;
	if(!m_clientsocketsql.Select(strSQL, vvResult))
	{
		MessageBox("Select Failed!");
	}

	if(vvResult.empty()) return;

	//����һ�е����ݲ�����ͷ
	for (int i = 0; i != vvResult[0].size(); i++)
	{
		m_lcStudent.InsertColumn(i, vvResult[0][i].c_str(), LVCFMT_LEFT, 150);
	}
	
	//�������������Ȳ����һ�еĵ�һ�����ݣ���set��������
	int nCurRow = 0;
	for (int i = 1; i != vvResult.size(); i++)
	{
		for (int j = 0; j != vvResult[i].size(); j++)
		{
			if(j == 0) nCurRow = m_lcStudent.InsertItem(0, vvResult[i][j].c_str());
			else
			{
				m_lcStudent.SetItemText(nCurRow, j, vvResult[i][j].c_str());
			}
		}
	}
}


void CClientSocketDlg::OnBnClickedBtninsert()
{
	NoSelect();
}


void CClientSocketDlg::OnBnClickedBtnupdate()
{
	NoSelect();
}


void CClientSocketDlg::OnBnClickedBtndelete()
{
	NoSelect();
}

void CClientSocketDlg::NoSelect()
{
	char strSQL[CClientSocketSQL::BUFFER_SIZE + 1] = {0};
	GetDlgItemText(IDC_EDITSQL, strSQL, CClientSocketSQL::BUFFER_SIZE);
	SetDlgItemText(IDC_EDITSQL, "");

	if(strlen(strSQL) == 0) return;

	char strResult[CClientSocketSQL::BUFFER_SIZE] = {0};
	m_clientsocketsql.NoSelect(strSQL, strResult);

	MessageBox(strResult);
}