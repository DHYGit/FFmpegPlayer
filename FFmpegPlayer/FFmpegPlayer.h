
// FFmpegPlayer.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CFFmpegPlayerApp:
// �йش����ʵ�֣������ FFmpegPlayer.cpp
//

class CFFmpegPlayerApp : public CWinApp
{
public:
	CFFmpegPlayerApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CFFmpegPlayerApp theApp;