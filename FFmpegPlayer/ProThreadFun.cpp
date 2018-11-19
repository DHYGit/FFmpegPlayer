#include "stdafx.h"
#include <conio.h>
#include "ProThreadFun.h"
#include "FFmpegTool.h"
#include "FFmpegPlayerDlg.h"
#include <queue>

int Play_FFmpeg(LPVOID pM)
{
	CFFmpegPlayerDlg *pDlg = (CFFmpegPlayerDlg*)pM;
	int n = pDlg->ffmpeg->FFmpeg_openFile((LPSTR)(LPCTSTR)pDlg->m_SourceFileName);
	if (n != 0){
		cprintf("Open source file failed\n");
		return n;
	}
	if(pDlg->ffmpeg->videoindex != -1 && pDlg->ffmpeg->audioindex != -1){
		pDlg->GetDlgItem(IDC_STATIC_Info)->SetWindowText("含有音频和视频");
	}else if(pDlg->ffmpeg->videoindex != -1 && pDlg->ffmpeg->audioindex == -1){
		pDlg->GetDlgItem(IDC_STATIC_Info)->SetWindowText("只有视频，没有音频");
	}else if(pDlg->ffmpeg->videoindex == -1 && pDlg->ffmpeg->audioindex != -1){
		pDlg->GetDlgItem(IDC_STATIC_Info)->SetWindowText("只有音频，没有视频");
	}else if(pDlg->ffmpeg->videoindex == -1 && pDlg->ffmpeg->audioindex == -1){
		pDlg->GetDlgItem(IDC_STATIC_Info)->SetWindowText("没有音频，也没有视频");
	}
	cprintf("ffmpeg cache buffer size:%d\n", pDlg->ffmpeg->pFormatCtx_In->pb->buffer_size);
	if(pDlg->ffmpeg->videoindex != -1){
		cprintf("SPS PPS: \n");
		for(int i = 0; i < pDlg->ffmpeg->pCodecCtxVideo_In->extradata_size; i++){
			cprintf("%02X  ", pDlg->ffmpeg->pCodecCtxVideo_In->extradata[i]);
		}
		cprintf("\n");
		pDlg->ffmpeg->bmp = SDL_CreateYUVOverlay(pDlg->ffmpeg->pCodecCtxVideo_In->width, pDlg->ffmpeg->pCodecCtxVideo_In->height, SDL_YV12_OVERLAY, pDlg->ffmpeg->screen);
	}
	cprintf("Open source file success \n");
	int video_index = 0;
	int audio_index = 0;
	int ret = 0;
	AVPacket pkt;
	std::queue <AVPacket> *video_pkt_buf_queue;
	std::queue <AVPacket> *audio_pkt_buf_queue;
	long video_pts = 0;
	long audio_pts = 0;
	pDlg->ffmpeg->start_time = av_gettime();
	while(1){
		if(pDlg->playStatus == 1){
			ret = av_read_frame(pDlg->ffmpeg->pFormatCtx_In, &pkt);
			if(pkt.stream_index == pDlg->ffmpeg->videoindex){
				video_pts = pkt.pts;
				cprintf("receive video data                                      pts %ld ,dts:%ld duration %d\n", pkt.pts, pkt.dts, pkt.duration);
				//预览
			    //AVRational time_base = pDlg->ffmpeg->pFormatCtx_In->streams[pDlg->ffmpeg->videoindex]->time_base;
				AVRational time_base = {1, 25};
				cprintf("time_base.num %d, time_base.den %d \n", time_base.num, time_base.den);
				AVRational time_base_q = {1, AV_TIME_BASE};
				int64_t pts_time = av_rescale_q(pkt.dts, time_base, time_base_q);
				int64_t now_time = av_gettime() - pDlg->ffmpeg->start_time;
				if (pts_time > now_time)
				{
					av_usleep(pts_time - now_time);
				}
				//Sleep(1000 * 1000 / time_base.den);
				pDlg->ffmpeg->FFmeg_PlayPacket(&pkt);
			}else{       
				audio_pts = pkt.pts;
				cprintf("receive audio data pCodecCtxAudio_In->sample_rate %d pts %ld, dts %ld duration %d\n",
					pDlg->ffmpeg->pCodecCtxAudio_In->sample_rate, pkt.pts, pkt.dts, pkt.duration);
				/*AVRational time_base = pDlg->ffmpeg->pFormatCtx_In->streams[pDlg->ffmpeg->audioindex]->time_base;
				AVRational time_base_q = {1, AV_TIME_BASE};
				int64_t pts_time = av_rescale_q(pkt.dts, time_base, time_base_q);
				int64_t now_time = av_gettime() - pDlg->ffmpeg->start_time;
				if (pts_time > now_time)rtmp://rtmp-test.rabbitslive.com/test1/00044b6657b3?t=5b88020c&k=d282a947e94ee22254824749922efaf9
				{
					av_usleep(pts_time - now_time);
				}*/
				pDlg->ffmpeg->packet_queue_put(&pDlg->ffmpeg->audioq, &pkt);
			}
			//av_packet_unref(&pkt);
		}
	}
	return 0;
}