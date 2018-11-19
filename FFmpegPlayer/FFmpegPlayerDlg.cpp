
// FFmpegPlayerDlg.cpp : ʵ���ļ�
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

// CFFmpegPlayerDlg �Ի���

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

// CFFmpegPlayerDlg ��Ϣ�������

BOOL CFFmpegPlayerDlg::OnInitDialog()
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
	AllocConsole();//�򿪿���̨
	cprintf("open console\n");
	InitFFmpeg();
	InitSDL();
	openSourceFileStatus = 0;
	playStatus = 0;
	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
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

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CFFmpegPlayerDlg::OnPaint()
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
int CFFmpegPlayerDlg::InitFFmpeg(){
	ffmpeg = new FFmpegClass();
	ffmpeg->FFmpeg_Init();
	return 0;
}
int CFFmpegPlayerDlg::InitSDL(){
	HWND hWnd = this->GetDlgItem(IDC_STATIC_Player)->GetSafeHwnd();
	if (hWnd != NULL) {
		char sdl_var[64];
		sprintf_s(sdl_var, "SDL_WINDOWID=%d", hWnd); //����һ�������пո�SDL_WINDOWID=%d"
		SDL_putenv(sdl_var);
		char *myvalue = SDL_getenv("SDL_WINDOWID");//��SDL��ȡ����ID
	}
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {
		printf("���ܳ�ʼ��SDL--%s\n", SDL_GetError());
		return -1;
	}
	//����SDL�¼�״̬
	SDL_EventState(SDL_ACTIVEEVENT, SDL_IGNORE);
	SDL_EventState(SDL_SYSWMEVENT, SDL_IGNORE);
	SDL_EventState(SDL_USEREVENT, SDL_IGNORE);
	//��ȡStatic�ؼ��Ŀ�Ⱥ͸߶�
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
//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CFFmpegPlayerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CFFmpegPlayerDlg::OnBnClickedButtonOpen()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	if(openSourceFileStatus){
		MessageBox(_T("����ֹͣ��ǰ�򿪵��ļ�"));
		return;
	}
	CString szFilter = _T("All Files (*.*)|*.*|avi Files (*.avi)|*.avi|rmvb Files (*.rmvb)|*.rmvb|3gp Files (*.3gp)|*.3gp|mp3 Files (*.mp3)|*.mp3|mp4 Files (*.mp4)|*.mp4|mpeg Files (*.ts)|*.ts|flv Files (*.flv)|*.flv|mov Files (*.mov)|*.mov||");
	CFileDialog OpenDlg(TRUE,NULL ,NULL,OFN_PATHMUSTEXIST|OFN_HIDEREADONLY ,szFilter,NULL); 
	if(IDOK == OpenDlg.DoModal()){
		//����Դ�ļ�
		m_SourceFileName = OpenDlg.GetPathName();
		//�༭����ʾ�ļ�·��
		this->GetDlgItem(IDC_EDIT_URL)->SetWindowText(this->m_SourceFileName);
		UpdateData(FALSE);
	}
}

void CFFmpegPlayerDlg::OnBnClickedButtonPlay()
{
	// TODO: �ڴ���ӿؼ�֪ͨ����������
	this->GetDlgItem(IDC_EDIT_URL)->GetWindowText(this->m_SourceFileName);
	if(m_SourceFileName == ""){
		MessageBox(_T("������URL����ļ�"));
		return;
	}
	CString str;
	this->GetDlgItem(IDC_BUTTON_Play)->GetWindowText(str);
	if(str == "����"){
		DWORD PlayThreadID;
		HANDLE PlayThread = CreateThread(NULL, 0,(LPTHREAD_START_ROUTINE)Play_FFmpeg, this, 0, &PlayThreadID);
		if(NULL == PlayThreadID){
			MessageBox(_T("create play thread failed"));
			return;
		}
	    this->playStatus = 1;
		this->GetDlgItem(IDC_BUTTON_Play)->SetWindowText("ֹͣ");
	}else{
		this->playStatus = 0;
		this->GetDlgItem(IDC_BUTTON_Play)->SetWindowText("����");
	}
}
