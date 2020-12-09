
// XWallPaperDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "XWallPaper.h"
#include "XWallPaperDlg.h"
#include "afxdialogex.h"
#include <iostream>
#include <stdio.h>


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CXWallPaperDlg 对话框



CXWallPaperDlg::CXWallPaperDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_XWALLPAPER_DIALOG, pParent)
	, m_LoopType(0)
	, m_DrawType(0)
	, m_VTimeOut(2)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CXWallPaperDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_CBIndex(pDX, IDC_COMBOLOOPTYPE, m_LoopType);
	DDX_CBIndex(pDX, IDC_COMBODRAWTYPE, m_DrawType);
	DDX_Control(pDX, IDC_LISTFILES, m_FileList);
	DDX_CBIndex(pDX, IDC_COMBO1, m_VTimeOut);
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam)
{
	HWND p = FindWindowEx(hwnd, NULL, "SHELLDLL_DefView", NULL);
	HWND* ret = (HWND*)lParam;

	if (p)
	{
		// Gets the WorkerW Window after the current one.
		*ret = FindWindowEx(NULL, hwnd, "WorkerW", NULL);
	}
	return true;
}

HWND CXWallPaperDlg::findDesktopWnd()
{
	//HWND hdesk = NULL;
	//HWND  hwndParent = NULL;
	//HWND  hwndSHELLDLL_DefView = NULL;
	//do
	//{
	//	hwndParent = ::FindWindowEx(NULL, hwndParent, "WorkerW", "");
	//	hwndSHELLDLL_DefView = ::FindWindowEx(hwndParent, NULL, "SHELLDLL_DefView", NULL);
	//	hdesk = ::FindWindowEx(hwndSHELLDLL_DefView, NULL, "SysListView32", "FolderView");
	//} while (hdesk == NULL);


	//HWND  hwndParent = ::FindWindow(L"Progman", L"Program Manager");
	//if (hwndParent == NULL)
	//{
	//	return NULL;
	//}
	//HWND  hwndSHELLDLL_DefView = ::FindWindowEx(hwndParent, NULL, L"SHELLDLL_DefView", NULL);
	//HWND  hwndSysListView32 = ::FindWindowEx(hwndSHELLDLL_DefView, NULL, L"SysListView32", L"FolderView");

	 // Fetch the Progman window
	HWND progman = ::FindWindow("ProgMan", NULL);
	// Send 0x052C to Progman. This message directs Progman to spawn a 
	// WorkerW behind the desktop icons. If it is already there, nothing 
	// happens.
	SendMessageTimeout(progman, 0x052C, 0, 0, SMTO_NORMAL, 1000, nullptr);
	// We enumerate all Windows, until we find one, that has the SHELLDLL_DefView 
	// as a child. 
	// If we found that window, we take its next sibling and assign it to workerw.
	HWND wallpaper_hwnd = nullptr;
	EnumWindows(EnumWindowsProc, (LPARAM)&wallpaper_hwnd);
	// Return the handle you're looking for.
	return wallpaper_hwnd;

}

BEGIN_MESSAGE_MAP(CXWallPaperDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTNSELFOLDER, &CXWallPaperDlg::OnBnClickedBtnselfolder)
	ON_BN_CLICKED(IDC_BTNADDFILES, &CXWallPaperDlg::OnBnClickedBtnaddfiles)
	ON_BN_CLICKED(IDC_BTNCLEANLIST, &CXWallPaperDlg::OnBnClickedBtncleanlist)
	ON_BN_CLICKED(IDCANCEL, &CXWallPaperDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDOK, &CXWallPaperDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BTNEXIT, &CXWallPaperDlg::OnBnClickedBtnexit)
	ON_CBN_SELCHANGE(IDC_COMBOLOOPTYPE, &CXWallPaperDlg::OnCbnSelchangeCombolooptype)
	ON_CBN_SELCHANGE(IDC_COMBODRAWTYPE, &CXWallPaperDlg::OnCbnSelchangeCombodrawtype)
	ON_MESSAGE(WM_X_ICONNOTIFY, &CXWallPaperDlg::OnXIconnotify)
	ON_BN_CLICKED(IDC_BTNLAST, &CXWallPaperDlg::OnBnClickedBtnlast)
	ON_BN_CLICKED(IDC_BTNNEXT, &CXWallPaperDlg::OnBnClickedBtnnext)
END_MESSAGE_MAP()


// CXWallPaperDlg 消息处理程序

BOOL CXWallPaperDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	ReadFileList();
	if (!m_PlayFiles.IsEmpty())
	{
		StartPlay();
	}
	NOTIFYICONDATA na;
	na.cbSize = (DWORD)sizeof(NOTIFYICONDATA);
	na.hWnd = this->m_hWnd;
	na.uID = IDR_MAINFRAME;
	na.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	na.uCallbackMessage = WM_X_ICONNOTIFY;
	na.hIcon = m_hIcon;
	strcpy(na.szTip, "双击显示窗口");
	Shell_NotifyIcon(NIM_ADD,&na);
	
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CXWallPaperDlg::OnPaint()
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
HCURSOR CXWallPaperDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


//
//void CXWallPaperDlg::OnBnClickedBtnstart()
//{
//	// TODO: 在此添加控件通知处理程序代码
//	if (m_PlayWnd == nullptr)
//	{
//		m_PlayWnd = new XPlayWnd();
//		RECT r;
//		CWnd * wallWnd = CWnd::FromHandle(findDesktopWnd());
//		wallWnd->GetWindowRect(&r);
//		m_PlayWnd->CreateEx(0,NULL, "", WS_VISIBLE| WS_CHILD, r, wallWnd, 10000);
//		m_PlayWnd->ShowWindow(SW_SHOW);
//		m_PlayWnd->Play("D:\\coolvoice_data\\小猪佩奇.mp4");
//	}
//}


void CXWallPaperDlg::OnBnClickedBtnselfolder()
{
	// TODO: 在此添加控件通知处理程序代码
	CFolderPickerDialog dlg(NULL, 0, this);
	if (IDOK != dlg.DoModal())
	{
		return;
	}
	CString folder = dlg.GetFolderPath();
	//遍历文件
	folder += "\\";
	CFileFind finder;
	BOOL IsFind = finder.FindFile(folder + _T("*.*"));

	while (IsFind)
	{
		IsFind = finder.FindNextFile();
		if (finder.IsDots() || finder.IsDirectory())
		{
			continue;
		}
		else
		{
			CString fileName = finder.GetFileName();
			CString filePath = finder.GetFilePath();
			CString ext = fileName.Right(fileName.GetLength() - fileName.ReverseFind('.'));
			if (ext == ".mp4" 
				|| ext == ".webm"
				|| ext == ".avi"
				|| ext == ".rm"
				|| ext == ".rmvb"
				|| ext == ".mpeg")
			{
				m_Files.AddTail(filePath);
				m_FileList.AddString(fileName);
			}
		}
	}
}


void CXWallPaperDlg::OnBnClickedBtnaddfiles()
{
	// TODO: 在此添加控件通知处理程序代码
	CString filter = _T("Video Files (*.mp4;*.webm;*.avi;*.rm;*.rmvb)|*.mp4;*.webm;*.avi;*.rm;*.rmvb||");
	CFileDialog dlg(TRUE,"","", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, filter);
	if (IDOK != dlg.DoModal())
	{
		return;
	}
	CString fileName = dlg.GetFileName();
	CString filePath = dlg.GetPathName();
	m_Files.AddTail(filePath);
	m_FileList.AddString(fileName);

}


void CXWallPaperDlg::OnBnClickedBtncleanlist()
{
	// TODO: 在此添加控件通知处理程序代码
	m_Files.RemoveAll();
	m_PlayPos = NULL;
	m_PlayFiles.RemoveAll();
	m_FileList.ResetContent();
}


void CXWallPaperDlg::OnBnClickedCancel()
{
	// TODO: 在此添加控件通知处理程序代码
	//CDialogEx::OnCancel();
	ShowWindow(SW_HIDE);
	m_bHide = true;
	m_Files.RemoveAll();
	m_FileList.ResetContent();
	auto pos = m_PlayFiles.GetHeadPosition();
	while (pos != NULL)
	{
		CString str = m_PlayFiles.GetNext(pos);
		//m_Files.AddTail(str);
		m_FileList.AddString(str);
	}

}


void CXWallPaperDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	//CDialogEx::OnOK();
	UpdateData();
	ShowWindow(SW_HIDE);
	m_bHide = true;

	auto pos = m_Files.GetHeadPosition();
	while (pos != NULL)
	{
		m_PlayFiles.AddTail(m_Files.GetNext(pos));
	}
	SaveFileList();
	StartPlay();
}


void CXWallPaperDlg::OnBnClickedBtnexit()
{
	// TODO: 在此添加控件通知处理程序代码
	m_PlayWnd->ShowWindow(SW_HIDE);
	m_PlayWnd->Stop();
	CDialogEx::OnOK();

}

void CXWallPaperDlg::ReadFileList()
{
	TCHAR buf[MAX_PATH + 1];
	GetModuleFileName(NULL, buf, MAX_PATH);
	_tcsrchr(buf, '\\')[1] = 0;
	_tcscat_s(buf, "list.xd");
	CFile f;
	if (f.Open(buf, CFile::modeRead))
	{
		int len = 0;
		f.Read(&len, 4);
		while (len)
		{
			char buf[MAX_PATH] = {0};
			f.Read(buf, len);
			CString str = buf;
			m_PlayFiles.AddTail(str);
			//m_Files.AddTail(str);
			m_FileList.AddString(str.Right(str.GetLength() - str.ReverseFind('\\') - 1));
			if (!f.Read(&len, 4))
				break;
		}
		f.Close();
	}
}

void CXWallPaperDlg::SaveFileList()
{
	TCHAR buf[MAX_PATH + 1];
	GetModuleFileName(NULL, buf, MAX_PATH);
	_tcsrchr(buf, '\\')[1] = 0;
	_tcscat_s(buf, "list.xd");
	CFile f;
	if (f.Open(buf,CFile::modeWrite | CFile::modeCreate))
	{
		auto pos = m_PlayFiles.GetHeadPosition();
		while (pos != NULL)
		{
			CString str =m_PlayFiles.GetNext(pos);
			int len = str.GetLength();
			f.Write(&len, 4);
			f.Write(str.GetBuffer(), str.GetLength());
		}
		f.Close();
	}
}

void CXWallPaperDlg::StartPlay()
{
	
	if (m_PlayWnd == nullptr)
	{
		srand(time(NULL));
		m_PlayPos = m_PlayFiles.GetHeadPosition();
		m_PlayWnd = new XPlayWnd();
		RECT r;
		m_LastTime = time(NULL);
		CWnd * wallWnd = CWnd::FromHandle(findDesktopWnd());
		wallWnd->GetWindowRect(&r);
		m_PlayWnd->CreateEx(0, NULL, "", WS_VISIBLE | WS_CHILD, r, wallWnd, 10000);
		m_PlayWnd->SetDrawType(m_DrawType);
		m_PlayWnd->ShowWindow(SW_SHOW);
		m_PlayWnd->SetFinishCB([this]() {
			while (m_PlayFiles.IsEmpty())
			{
				Sleep(300);
			}
			if (time(NULL) - m_LastTime > (m_VTimeOut+1) * 60)
			{
				m_LastTime = time(NULL);
				switch (m_LoopType)
				{
				case 0:
					m_PlayFiles.GetNext(m_PlayPos);
					if (m_PlayPos == NULL)
					{
						m_PlayPos = m_PlayFiles.GetHeadPosition();
					}
					break;
				case 1:
					//m_PlayFiles.GetPrev(m_PlayPos);
					//m_PlayFiles.GetNext(m_PlayPos);
					break;
				case 2:
				{
					int r = rand() % m_PlayFiles.GetCount() + 1;
					while (r > 0)
					{
						m_PlayFiles.GetNext(m_PlayPos);
						if (m_PlayPos == NULL)
						{
							m_PlayPos = m_PlayFiles.GetHeadPosition();
						}
						r--;
					}
				
				}
				break;
				default:
					break;
				}
			}
			if (m_PlayPos == NULL)
			{
				m_PlayPos = m_PlayFiles.GetHeadPosition();
			}
			m_PlayWnd->Play(m_PlayFiles.GetAt(m_PlayPos));
		});
		
		m_PlayWnd->Play(m_PlayFiles.GetAt(m_PlayPos));
	}
}


void CXWallPaperDlg::OnCbnSelchangeCombolooptype()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
}


void CXWallPaperDlg::OnCbnSelchangeCombodrawtype()
{
	// TODO: 在此添加控件通知处理程序代码
	UpdateData(TRUE);
	if(NULL != m_PlayWnd)
		m_PlayWnd->SetDrawType(m_DrawType);
}


afx_msg LRESULT CXWallPaperDlg::OnXIconnotify(WPARAM wParam, LPARAM lParam)
{
	UINT uMouseMsg = (UINT)lParam;
	if (uMouseMsg != WM_LBUTTONDBLCLK)
	{
		return 0;
	}
	if (m_bHide)
	{
		ShowWindow(SW_NORMAL);
		m_bHide = false;
	}
	else
	{
		ShowWindow(SW_HIDE);
		m_bHide = true;
	}
	return 0;
}


void CXWallPaperDlg::OnBnClickedBtnlast()
{
	// TODO: 在此添加控件通知处理程序代码
	m_LastTime = time(NULL);
	m_PlayFiles.GetPrev(m_PlayPos);
	if (m_PlayPos == NULL)
	{
		m_PlayPos = m_PlayFiles.GetTailPosition();
	}
	m_PlayWnd->Stop();
}


void CXWallPaperDlg::OnBnClickedBtnnext()
{
	// TODO: 在此添加控件通知处理程序代码
	m_LastTime = time(NULL);
	m_PlayFiles.GetNext(m_PlayPos);
	if (m_PlayPos == NULL)
	{
		m_PlayPos = m_PlayFiles.GetHeadPosition();
	}
	m_PlayWnd->Stop();
}
