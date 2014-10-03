
// LuaPlayerDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "LuaPlayer.h"
#include "LuaPlayerDlg.h"
#include "afxdialogex.h"
#include "LoadLua.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern "C"

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CLuaPlayerDlg 对话框



CLuaPlayerDlg::CLuaPlayerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CLuaPlayerDlg::IDD, pParent)
	, output(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CLuaPlayerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON1, button);
	DDX_Text(pDX, IDC_EDIT1, output);
}

BEGIN_MESSAGE_MAP(CLuaPlayerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CLuaPlayerDlg::OnBnClickedButton1)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CLuaPlayerDlg 消息处理程序

BOOL CLuaPlayerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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
	lua.InitLua();
	lua.SetHWND(this->GetSafeHwnd());
	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CLuaPlayerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CLuaPlayerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CLuaPlayerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CLuaPlayerDlg::OnBnClickedButton1()
{
	static int sw;
	if(sw)
	{
		button.SetWindowTextW(_T("打开脚本"));
		lua.CloseAllTimer();
		sw=0;
		return;
	}
	CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, _T("lua脚本文件 (*.lua)|*.lua|所有文件 (*.*)|*.*||"), NULL);
	if(dlg.DoModal()==IDOK)
	{
		if(lua.doFile(dlg.GetPathName()))
		{
			output=lua.GetErrorLog();
			UpdateData(false);
			AfxMessageBox(_T("运行错误"));
		}
		else
		{
			button.SetWindowTextW(_T("关闭脚本"));
			output=_T("");
			UpdateData(false);
			sw=1;
		}
	}
}


void CLuaPlayerDlg::OnTimer(UINT_PTR nIDEvent)
{
	lua.TimerCall(nIDEvent);
	output+=lua.GetErrorLog();
	UpdateData(false);
	CDialogEx::OnTimer(nIDEvent);
}


void CLuaPlayerDlg::UpdateLog(CString str)
{
	output+=str;
	UpdateData(false);
}
