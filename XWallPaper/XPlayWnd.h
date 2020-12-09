#pragma once
#include <afxwin.h>
#include <functional>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>
#include <libavutil/frame.h>
#include <libswresample/swresample.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#include <libavutil/opt.h>
#include <libavutil/error.h>
}

class XPlayWnd :
	public CWnd
{
	DECLARE_DYNAMIC(XPlayWnd)
public:
	XPlayWnd();
	~XPlayWnd();
	void SetFinishCB(std::function<void()> finishCb);
	void SetDrawType(int type);
	void Play(CString file);
	void Stop();
private:
	static DWORD WINAPI ThreadProc(_In_ LPVOID lpParameter	);
	int ShowInDlg(AVFrame *pFrameRGB, int width, int height, int bpp);
	void FillBluck();
protected:
	//afx_msg void OnPaint();

	DECLARE_MESSAGE_MAP()
private:
	CString m_VideoFile;
	//循环播放
	bool m_bLoop = true;
	bool m_bStop = false;
	HBITMAP m_hbitmap;
	int m_vWidth;
	int m_vHeight;
	//画面比例 ，0适应桌面，1原始尺寸，2比例缩放
	int m_DrawType = 0;
	std::function<void()> m_finishCb;
};

