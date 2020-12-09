#include "stdafx.h"
#include "XPlayWnd.h"

IMPLEMENT_DYNAMIC(XPlayWnd, CWnd)

XPlayWnd::XPlayWnd()
{
	//AfxRegisterWndClass(L"XPlayWnd");
}


XPlayWnd::~XPlayWnd()
{
}

void XPlayWnd::SetFinishCB(std::function<void()> finishCb)
{
	m_finishCb = finishCb;
}

void XPlayWnd::SetDrawType(int type)
{
	m_DrawType = type;
}

void XPlayWnd::Play(CString file)
{
	m_bStop = false;
	m_VideoFile = file;
	CreateThread(NULL, NULL, ThreadProc, this, 0, 0);

}

void XPlayWnd::Stop()
{
	m_bLoop = false;
	m_bStop = true;
}

DWORD __stdcall XPlayWnd::ThreadProc(LPVOID lpParameter)
{
	XPlayWnd* dlg = static_cast<XPlayWnd*>(lpParameter);
	dlg->FillBluck();
	CRect r;
	dlg->GetWindowRect(r);
	//����Ŀ��ߴ�
	int dw, dh;

	AVFormatContext *pFormatCtx = nullptr;
	//int	audioindex = -1;
	int videoindex = -1;
	AVCodecContext *pCodecCtx = nullptr;
	AVCodec *pCodec = nullptr;
	AVFrame *pFrame = nullptr;
	AVFrame	*pFrameRGB = nullptr;
	unsigned char *out_buffer = nullptr;
	AVPacket *packet = nullptr;
	int ret, got_picture;
	struct SwsContext *img_convert_ctx = nullptr;
	char filepath[_MAX_PATH] = { 0 };
	INT64 dieTime = GetTickCount64();
	int LastFreamDely = 0;			//��һ֡�ĳ���ʱ��
	int defaultDely = 40;			//Ĭ��ÿ֡��ʱ
	int AudioRate = 0;
	int AudioChannel = 0;
	//AVCodecContext *AudioCodec = nullptr;
	double time_base = 0;			//����ʱ�䵥λ��������
	//SwrContext *m_SWRtx = nullptr;

	//��ʼ��������
	av_register_all();//����AVFormatContext������������صĽṹ��
	pFormatCtx = avformat_alloc_context();
	//��ʼ��pFormatCtx�ṹ
	if (avformat_open_input(&pFormatCtx, dlg->m_VideoFile.GetBuffer(), NULL, NULL) != 0) {
		OutputDebugString("Couldn't open input stream.\n");
		goto end;
	}
	//��ȡ����Ƶ��������Ϣ
	if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
		OutputDebugString("Couldn't find stream information.\n");
		goto end;
	}
	//nb_streams����Ƶ���ĸ��������ﵱ���ҵ���Ƶ��ʱ���ж��ˡ�
	for (int i = 0; i < pFormatCtx->nb_streams; i++)
	{
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoindex = i;
			break;
		}
		//else if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)
		//{
		//	audioindex = i;
		//}
	}

	if (videoindex == -1) {
		OutputDebugString("Didn't find a video stream.\n");
		goto end;
	}
	/*if (audioindex == -1) {
		OutputDebugString("Didn't find a Audio stream.\n");
		goto end;
	}*/
	//��ȡ��Ƶ����Ϣ
	//AudioCodec = pFormatCtx->streams[audioindex]->codec;
	// ������Ƶ������
	//AVCodec *codec = avcodec_find_decoder(AudioCodec->codec_id);
	//if (codec == nullptr)
	//	goto end;

	// ����Ƶ������
	//if (avcodec_open2(AudioCodec, codec, nullptr) != 0)
	//	goto end;
	//AudioCodec->channel_layout = av_get_default_channel_layout(AudioCodec->channels);

	//int rate = AudioCodec->sample_rate;
	//int channel = AudioCodec->channels;
	//g_AudioPlayThread->cleanAllAudioBuffer();
	//g_AudioPlayThread->setCurrentSampleInfo(rate, 16, channel);
	//g_AudioPlayThread->start();
	//��ȡ��Ƶ������ṹ
	pCodecCtx = pFormatCtx->streams[videoindex]->codec;
	//���ҽ�����
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL) {
		OutputDebugString("Codec not found.\n");
		goto end;
	}
	//���ڳ�ʼ��pCodecCtx�ṹ
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
		OutputDebugString("Could not open codec.\n");
		goto end;
	}
	//����֡�ṹ���˺�������������ṹ�ռ䣬ͼ�����ݿռ���ͨ��av_malloc����
	pFrame = av_frame_alloc();
	pFrameRGB = av_frame_alloc();
	switch (dlg->m_DrawType)
	{
	case 0:
		dw = r.Width();
		dh = r.Height();
		break;
	case 1:
		dw = pCodecCtx->width;
		dh = pCodecCtx->height;
		break;
	case 2:
		if (1.0 * r.Width() / r.Height() >= 1.0 * pCodecCtx->width / pCodecCtx->height)
		{
			dh = r.Height();
			dw = pCodecCtx->width * (1.0 * r.Height() / pCodecCtx->height);
		}
		else
		{
			dw = r.Width();
			dh = pCodecCtx->height * (1.0 * r.Width() / pCodecCtx->width);
		}
		break;
	default:
		break;
	}
	//������̬�ڴ�,�����洢ͼ�����ݵĿռ�
	//av_image_get_buffer_size��ȡһ֡ͼ����Ҫ�Ĵ�С
	out_buffer = (unsigned char *)av_malloc(av_image_get_buffer_size(AV_PIX_FMT_RGB32, dw,dh, 1));
	av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, out_buffer,
		AV_PIX_FMT_RGB32, dw, dh, 1);

	packet = (AVPacket *)av_malloc(sizeof(AVPacket));

	//ͨ��ƽ��֡�ʻ�ȡĬ��ʱ��
	{
		int num = pFormatCtx->streams[videoindex]->avg_frame_rate.num;
		int den = pFormatCtx->streams[videoindex]->avg_frame_rate.den;
		defaultDely = 1000 * den / num;
		num = pFormatCtx->streams[videoindex]->time_base.num;
		den = pFormatCtx->streams[videoindex]->time_base.den;
		time_base = 1000.0 * num / den;
	}

	//m_SWRtx = swr_alloc();
	//swr_alloc_set_opts(m_SWRtx, AudioCodec->channel_layout, AV_SAMPLE_FMT_S16, \
	//	AudioCodec->sample_rate, AudioCodec->channels, AudioCodec->sample_fmt, \
	//	AudioCodec->sample_rate, 0, 0);
	//swr_init(m_SWRtx);

	//��ʼ��img_convert_ctx�ṹ
	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
		dw, dh, AV_PIX_FMT_RGB32, SWS_BICUBIC, NULL, NULL, NULL);
	dlg->m_vWidth = pCodecCtx->width;
	dlg->m_vHeight = pCodecCtx->height;

	//av_read_frame��ȡһ֡δ���������
	while (av_read_frame(pFormatCtx, packet) >= 0) {
		
		//�������Ƶ����
		if (packet->stream_index == videoindex) {
			// ������Ƶ֡�� ������Ƶ��
			ret = avcodec_send_packet(pCodecCtx, packet);
			if (ret != 0)
				break;

			// ������Ƶ֡��������Ƶ����֡
			ret = avcodec_receive_frame(pCodecCtx, pFrame);
			if (ret == 0) {
				//��תͼ�� ���������ɵ�ͼ�������µ�����  
				pFrame->data[0] += pFrame->linesize[0] * (pCodecCtx->height - 1);
				pFrame->linesize[0] *= -1;
				pFrame->data[1] += pFrame->linesize[1] * (pCodecCtx->height / 2 - 1);
				pFrame->linesize[1] *= -1;
				pFrame->data[2] += pFrame->linesize[2] * (pCodecCtx->height / 2 - 1);
				pFrame->linesize[2] *= -1;

				//ת��ͼ���ʽ������ѹ������YUV420P��ͼ��ת��ΪBRG24��ͼ��  
				ret = sws_scale(img_convert_ctx, pFrame->data,
					pFrame->linesize, 0, pCodecCtx->height,
					pFrameRGB->data, pFrameRGB->linesize);

				//�ȴ�ǰһ֡�ĳ����¼�
				while (GetTickCount64() < dieTime)
				{
					if (dlg->m_bStop)
					{
						goto end;
					}
					Sleep(1);
				}
				dlg->ShowInDlg(pFrameRGB, dw, dh,32);

			}
			LastFreamDely = packet->duration * time_base;
			if (LastFreamDely == 0)
			{
				LastFreamDely = defaultDely;
			}

			dieTime += LastFreamDely;
		}
		//else if (packet->stream_index == audioindex)
		//{
		//	//// ������Ƶ֡�� ������Ƶ��
		//	//ret = avcodec_send_packet(AudioCodec, packet);
		//	//if (ret != 0)
		//	//	break;

		//	//// ������Ƶ֡��������Ƶ����֡
		//	//ret = avcodec_receive_frame(AudioCodec, pFrame);
		//	//if (ret != 0)
		//	//	continue;


		//	//uint8_t *array[1];
		//	//uint8_t arrays[10000] = { 0 };
		//	//array[0] = arrays;
		//	//int len = swr_convert(m_SWRtx, array, 10000, (const uint8_t **)pFrame->data, pFrame->nb_samples);

		//	////g_AudioPlayThread->addAudioBuffer((char*)arrays, m_AvFrame->linesize[0]);
		//	//g_AudioPlayThread->addAudioBuffer((char*)arrays, pFrame->linesize[0]);

		//}
		av_free_packet(packet);
	}

end:
	if (img_convert_ctx)
		sws_freeContext(img_convert_ctx);
	if (pFrameRGB)
		av_frame_free(&pFrameRGB);
	if (pFrame)
		av_frame_free(&pFrame);
	if (pCodecCtx)
		avcodec_close(pCodecCtx);
	if (out_buffer)
		av_free(out_buffer);
	//if (AudioCodec)
	//	avcodec_close(AudioCodec);
	//if (m_SWRtx)
	//	swr_free(&m_SWRtx);
	if (pFormatCtx)
		avformat_close_input(&pFormatCtx);
	dlg->m_finishCb();
	return 0;
}

int XPlayWnd::ShowInDlg(AVFrame * pFrameRGB, int width, int height, int bpp)
{
	BITMAPFILEHEADER bmpheader;
	BITMAPINFOHEADER bmpinfo;
	FILE *fp;
	bmpheader.bfType = 0x4d42;
	bmpheader.bfReserved1 = 0;
	bmpheader.bfReserved2 = 0;
	bmpheader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);
	bmpheader.bfSize = bmpheader.bfOffBits + width * height*bpp / 8;

	bmpinfo.biSize = sizeof(BITMAPINFOHEADER);
	bmpinfo.biWidth = width;
	bmpinfo.biHeight = height;
	bmpinfo.biPlanes = 1;
	bmpinfo.biBitCount = bpp;
	bmpinfo.biCompression = BI_RGB;
	bmpinfo.biSizeImage = (width*bpp + 31) / 32 * 4 * height;
	bmpinfo.biXPelsPerMeter = 100;
	bmpinfo.biYPelsPerMeter = 100;
	bmpinfo.biClrUsed = 0;
	bmpinfo.biClrImportant = 0;
	if (m_hbitmap)
		::DeleteObject(m_hbitmap);

	CDC* dc = GetDC();
	
	m_hbitmap = CreateDIBitmap(dc->GetSafeHdc(),							//�豸�����ĵľ�� 
		(LPBITMAPINFOHEADER)&bmpinfo,				//λͼ��Ϣͷָ�� 
		(long)CBM_INIT,								//��ʼ����־ 
		pFrameRGB->data[0],						//��ʼ������ָ�� 
		(LPBITMAPINFO)&bmpinfo,						//λͼ��Ϣָ�� 
		DIB_RGB_COLORS);

	CDC memDc;
	memDc.CreateCompatibleDC(dc);
	
	CRect rcClient;
	GetWindowRect(rcClient);
	
	memDc.SelectObject(m_hbitmap);
	int nleft = (rcClient.Width() - width) / 2;
	int top = (rcClient.Height() - height) / 2;
	dc->StretchBlt(nleft, top, width, height, &memDc, 0, 0, width, height, SRCCOPY);

	memDc.DeleteDC();

	ReleaseDC(dc);
	return 0;
}

void XPlayWnd::FillBluck()
{
	if (m_DrawType == 0)
	{
		return;
	}
	CDC* dc = GetDC();
	CRect rcClient;
	GetWindowRect(rcClient);
	CBrush *bluck = new CBrush(RGB(0, 0, 0));
	dc->FillRect(rcClient, bluck);
	delete bluck;
}

BEGIN_MESSAGE_MAP(XPlayWnd, CWnd)
//	ON_WM_PAINT()
END_MESSAGE_MAP()

//void XPlayWnd::OnPaint()
//{
//}
