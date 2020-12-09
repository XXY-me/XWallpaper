
// XWallPaperDlg.h: 头文件
//

#pragma once
#include "XPlayWnd.h"

#define WM_X_ICONNOTIFY WM_USER+200

// CXWallPaperDlg 对话框
class CXWallPaperDlg : public CDialogEx
{
// 构造
public:
	CXWallPaperDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_XWALLPAPER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

	static HWND findDesktopWnd();
// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnXIconnotify(WPARAM wParam, LPARAM lParam);
public:
	afx_msg void OnBnClickedBtnselfolder();
	afx_msg void OnBnClickedBtnaddfiles();
	afx_msg void OnBnClickedBtncleanlist();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedBtnexit();
	afx_msg void OnCbnSelchangeCombolooptype();
	afx_msg void OnCbnSelchangeCombodrawtype();
	afx_msg void OnBnClickedBtnlast();
	afx_msg void OnBnClickedBtnnext();
private:
	void ReadFileList();
	void SaveFileList();
	void StartPlay();
private:
	// //循环类型，0列表循环，1单曲循环，2随机播放
	int m_LoopType = 0;
	// //画面比例，0适应桌面，1原始尺寸，2比例缩放
	int m_DrawType = 0;
	CList<CString> m_Files;
	CList<CString> m_PlayFiles;
	XPlayWnd* m_PlayWnd = nullptr;
	// 文件列表
	CListBox m_FileList;
	POSITION m_PlayPos;
	bool m_bHide = false;
	int m_VTimeOut;
	time_t m_LastTime = 0;
};
