
// FFmpegPlayerDlg.cpp : 实现文件
//
#include "stdafx.h"
#include "FFmpegPlayer.h"
#include "FFmpegPlayerDlg.h"
#include "afxdialogex.h"
#include "ProThreadFun.h"
#include <conio.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

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

// CFFmpegPlayerDlg 对话框

CFFmpegPlayerDlg::CFFmpegPlayerDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CFFmpegPlayerDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CFFmpegPlayerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CFFmpegPlayerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_Open, &CFFmpegPlayerDlg::OnBnClickedButtonOpen)
	ON_BN_CLICKED(IDC_BUTTON_Play, &CFFmpegPlayerDlg::OnBnClickedButtonPlay)
END_MESSAGE_MAP()

// CFFmpegPlayerDlg 消息处理程序

BOOL CFFmpegPlayerDlg::OnInitDialog()
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

	// 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	AllocConsole();//打开控制台
	cprintf("open console\n");
	InitFFmpeg();
	InitSDL();
	openSourceFileStatus = 0;
	playStatus = 0;
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CFFmpegPlayerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CFFmpegPlayerDlg::OnPaint()
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
int CFFmpegPlayerDlg::InitFFmpeg(){
	ffmpeg = new FFmpegClass();
	ffmpeg->FFmpeg_Init();
	return 0;
}
int CFFmpegPlayerDlg::InitSDL(){
	HWND hWnd = this->GetDlgItem(IDC_STATIC_Player)->GetSafeHwnd();
	if (hWnd != NULL) {
		char sdl_var[64];
		sprintf_s(sdl_var, "SDL_WINDOWID=%d", hWnd); //这里一定不能有空格SDL_WINDOWID=%d"
		SDL_putenv(sdl_var);
		char *myvalue = SDL_getenv("SDL_WINDOWID");//让SDL获取窗口ID
	}
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
		printf("不能初始化SDL--%s\n", SDL_GetError());
		return -1;
	}
	//设置SDL事件状态
	SDL_EventState(SDL_ACTIVEEVENT, SDL_IGNORE);
	SDL_EventState(SDL_SYSWMEVENT, SDL_IGNORE);
	SDL_EventState(SDL_USEREVENT, SDL_IGNORE);
	//获取Static控件的宽度和高度
	CRect rect;
	CWnd *pWnd = this->GetDlgItem(IDC_STATIC_Player);
	pWnd->GetClientRect(&rect);
	ffmpeg->screen = SDL_SetVideoMode(rect.Width(), rect.Height(), 0, 0);
	if (!ffmpeg->screen){
		printf("SDL: could not set video mode -exiting \n");
		return -1;
	}
	return 0;
}
//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CFFmpegPlayerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CFFmpegPlayerDlg::OnBnClickedButtonOpen()
{
	// TODO: 在此添加控件通知处理程序代码
	if(openSourceFileStatus){
		MessageBox(_T("请先停止当前打开的文件"));
		return;
	}
	CString szFilter = _T("All Files (*.*)|*.*|avi Files (*.avi)|*.avi|rmvb Files (*.rmvb)|*.rmvb|3gp Files (*.3gp)|*.3gp|mp3 Files (*.mp3)|*.mp3|mp4 Files (*.mp4)|*.mp4|mpeg Files (*.ts)|*.ts|flv Files (*.flv)|*.flv|mov Files (*.mov)|*.mov||");
	CFileDialog OpenDlg(TRUE,NULL ,NULL,OFN_PATHMUSTEXIST|OFN_HIDEREADONLY ,szFilter,NULL); 
	if(IDOK == OpenDlg.DoModal()){
		//加载源文件
		m_SourceFileName = OpenDlg.GetPathName();
		//编辑框显示文件路径
		this->GetDlgItem(IDC_EDIT_URL)->SetWindowText(this->m_SourceFileName);
		UpdateData(FALSE);
	}
}

void CFFmpegPlayerDlg::OnBnClickedButtonPlay()
{
	// TODO: 在此添加控件通知处理程序代码
	this->GetDlgItem(IDC_EDIT_URL)->GetWindowText(this->m_SourceFileName);
	if(m_SourceFileName == ""){
		MessageBox(_T("请输入URL或打开文件"));
		return;
	}
	CString str;
	this->GetDlgItem(IDC_BUTTON_Play)->GetWindowText(str);
	if(str == "播放"){
		DWORD PlayThreadID;
		HANDLE PlayThread = CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)Play_FFmpeg, this, 0, &PlayThreadID);
		if(NULL == PlayThreadID){
			MessageBox(_T("create play thread failed"));
			return;
		}
	    this->playStatus = 1;
		this->GetDlgItem(IDC_BUTTON_Play)->SetWindowText("停止");
	}else{
		this->playStatus = 0;
		this->GetDlgItem(IDC_BUTTON_Play)->SetWindowText("播放");
	}
}
