
// FFmpegPlayerDlg.h : 头文件
//

#pragma once
#include "FFmpegTool.h"
#include "Resource.h"

// CFFmpegPlayerDlg 对话框
class CFFmpegPlayerDlg : public CDialogEx
{
// 构造
public:
	CFFmpegPlayerDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
	enum { IDD = IDD_FFMPEGPLAYER_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持
public:
	FFmpegClass *ffmpeg;
	int InitFFmpeg();
	int InitSDL();
	int openSourceFileStatus;
	int playStatus;//1播放, 2停止播放
	CString m_SourceFileName;
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
	afx_msg void OnBnClickedButtonOpen();
	afx_msg void OnBnClickedButtonPlay();
};
