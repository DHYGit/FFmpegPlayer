
// FFmpegPlayerDlg.h : ͷ�ļ�
//

#pragma once
#include "FFmpegTool.h"
#include "Resource.h"

// CFFmpegPlayerDlg �Ի���
class CFFmpegPlayerDlg : public CDialogEx
{
// ����
public:
	CFFmpegPlayerDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
	enum { IDD = IDD_FFMPEGPLAYER_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��
public:
	FFmpegClass *ffmpeg;
	int InitFFmpeg();
	int InitSDL();
	int openSourceFileStatus;
	int playStatus;//1����, 2ֹͣ����
	CString m_SourceFileName;
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
	afx_msg void OnBnClickedButtonOpen();
	afx_msg void OnBnClickedButtonPlay();
};
