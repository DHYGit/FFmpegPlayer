#include "stdafx.h"
#include "FFmpegTool.h"
#include <windows.h>
#include <stdio.h>
#include <time.h>
#include <conio.h>

HANDLE hMutex = CreateMutex(NULL,FALSE,NULL);

#define CLOCK_TIME_BASE     90000

static AVFrame *picture, *tmp_picture;  
static uint8_t *video_outbuf;  
static int frame_count, video_outbuf_size;  

static int num = 0;
static int index = 0;
FILE *pYUVFile;
FILE *outputFile;
static char* fileName[5] = {"opencvyuv1.yuv", "opencvyuv2.yuv", "opencvyuv3.yuv", "opencvyuv4.yuv", "opencvyuv5.yuv"};

//ʱ���
clock_t tick1, tick2;
FFmpegClass *ffmpeg;
void FFmpegClass::FFmpeg_Init() {
	av_register_all();
	avformat_network_init();
	pFormatCtx_In = avformat_alloc_context();
	pFrameDec = av_frame_alloc();
	pFrameYUV = av_frame_alloc(); 
	pFrameEnc = av_frame_alloc();	
	pFramePCM = av_frame_alloc();
}

void FFmpegClass::packet_queue_init(PacketQueue *q) {
    memset(q, 0, sizeof(PacketQueue));
    q->mutex = SDL_CreateMutex();
    q->cond = SDL_CreateCond();
}

FFmpegClass::FFmpegClass()
{
	ffmpeg = this;
}

int FFmpegClass::FFmpeg_openFile(char *fileName) {
	AVDictionary *dic = NULL;
	int res = av_dict_set(&dic, "stimeout", "2000000", 0);
	res = av_dict_set(&dic, "bufsize", "655360", 0);
	int t = avformat_open_input(&pFormatCtx_In, fileName, NULL, &dic);
	if (t != 0) {
		printf("���ļ�ʧ��\n");
		return -1;
	}
	if (avformat_find_stream_info(pFormatCtx_In, NULL)<0){
		printf("�Ҳ�������Ϣ");
		return -2;
	}
	videoindex = -1;
	//������е�����������������ΪAVMEDIA_TYPE_VIDEO
	for (int i = 0; i < pFormatCtx_In->nb_streams; i++){
		if (pFormatCtx_In->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO){//�������Ƶ
			videoindex = i;
		}
	}
	audioindex = -1;
	//������е�����������������ΪAVMEDIA_TYPE_AUDIO
	for (int i = 0; i < pFormatCtx_In->nb_streams; i++){
		if (pFormatCtx_In->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO)//�������Ƶ
		{
			audioindex = i;
			break;
		}
	}
	if(videoindex != -1){
		pCodecCtxVideo_In = pFormatCtx_In->streams[videoindex]->codec;
		pCodecVideo_In = avcodec_find_decoder(pCodecCtxVideo_In->codec_id);//���Ҷ�Ӧ�Ľ�����
		if (pCodecVideo_In == NULL) {
			printf("������Ƶ������ʧ��\r\n");
			return -4;
		}
		//�򿪽�����
		if (avcodec_open2(pCodecCtxVideo_In,pCodecVideo_In, NULL) < 0) {
			printf("����Ƶ������ʧ��\r\n");
			return -5;
		}
		y_size = pCodecCtxVideo_In->width * pCodecCtxVideo_In->height;
		out_buffer = (uint8_t*)malloc(avpicture_get_size(AV_PIX_FMT_YUV420P, pCodecCtxVideo_In->width, pCodecCtxVideo_In->height));
		avpicture_fill((AVPicture *)pFrameYUV, out_buffer, AV_PIX_FMT_YUV420P, pCodecCtxVideo_In->width, pCodecCtxVideo_In->height);
		img_convert_ctx = sws_getContext(pCodecCtxVideo_In->width, pCodecCtxVideo_In->height, pCodecCtxVideo_In->pix_fmt, pCodecCtxVideo_In->width, pCodecCtxVideo_In->height, AV_PIX_FMT_YUV420P, SWS_BICUBIC, NULL, NULL, NULL);
	}else{
		printf("û�з�����Ƶ��\n");
	}
	if(audioindex != -1){
		pCodecCtxAudio_In = pFormatCtx_In->streams[audioindex]->codec;
		pCodecAudio_In = avcodec_find_decoder(pCodecCtxAudio_In->codec_id);//���Ҷ�Ӧ�Ľ�����
		if (pCodecAudio_In == NULL) {
			printf("������Ƶ������ʧ��\r\n");
			return -4;
		}
		//�򿪽�����
		if (avcodec_open2(pCodecCtxAudio_In,pCodecAudio_In, NULL) < 0) {
			printf("����Ƶ������ʧ��\r\n");
			return -5;
		}
		wanted_spec.freq = pCodecCtxAudio_In->sample_rate;//������
		//wanted_spec.freq = 48000;
		wanted_spec.format = AUDIO_S16SYS;//��Ƶ��ʽS:signed 16��ʾÿ������ռ16bits,SYS��ʾ��С�˺�ϵͳ����һ��
		wanted_spec.channels = pCodecCtxAudio_In->channels;//��Ƶͨ����
		wanted_spec.silence = 0;//��Ƶ�����б�ʾ������ֵ�Ƕ���
		wanted_spec.samples = 1024;//��Ƶ�����С
		wanted_spec.callback = audio_callback;//�ص�����
		wanted_spec.userdata = pCodecCtxAudio_In;//�ص����������û�����
		//pFramePCM->format = AV_SAMPLE_FMT_FLTP;
		//pFramePCM->format = AV_SAMPLE_FMT_S16P;
		pFramePCM->format = AV_SAMPLE_FMT_S16;
		pFramePCM->nb_samples = 1024;
		//pFramePCM->sample_rate = pCodecCtxAudio_In->sample_rate;
		pFramePCM->sample_rate = wanted_spec.freq;
		pFramePCM->channel_layout = AV_CH_LAYOUT_STEREO;
		pFramePCM->channels = pCodecCtxAudio_In->channels;
		pcm_size = av_samples_get_buffer_size(NULL, pCodecCtxAudio_In->channels,pCodecCtxAudio_In->frame_size,pCodecCtxAudio_In->sample_fmt, 0);
		this->convert_ctx = swr_alloc();
		convert_ctx = swr_alloc_set_opts(convert_ctx, 
			pFramePCM->channel_layout, 
			AV_SAMPLE_FMT_S16,
			pFramePCM->sample_rate, 
			pCodecCtxAudio_In->channel_layout,
			pCodecCtxAudio_In->sample_fmt,
			//pCodecCtxAudio_In->sample_rate,
			32000,
			0, NULL);
		swr_init(convert_ctx);
		if(SDL_OpenAudio(&wanted_spec, NULL) < 0){
			printf("sdl open audio failed \n");
			return -6;
		}
		packet_queue_init(&audioq);
		SDL_PauseAudio(0);
	}else{
		printf("û�з�����Ƶ��\n");
	}
	return 0;
}
void audio_callback(void *userdata, Uint8 *stream, int len){
	AVCodecContext *aCodecCtx = (AVCodecContext *)userdata;
    int len1, audio_size;
    
    static uint8_t audio_buf[(MAX_AUDIO_FRAME_SIZE * 3) / 2];
    static unsigned int audio_buf_size = 0;
    static unsigned int audio_buf_index = 0;
    
    while (len > 0) {
        if (audio_buf_index >= audio_buf_size) {
            // We have already sent all our data; get more.
            audio_size = ffmpeg->audio_decode_frame(aCodecCtx, audio_buf, audio_buf_size);
            if (audio_size < 0) {
                // If error, output silence.
                audio_buf_size = 1024; // arbitrary?
                memset(audio_buf, 0, audio_buf_size);
            } else {
                audio_buf_size = audio_size;
            }
            audio_buf_index = 0;
        }
        len1 = audio_buf_size - audio_buf_index;
        if (len1 > len) {
            len1 = len;
        }
        memcpy(stream, (uint8_t *) audio_buf + audio_buf_index, len1);
        len -= len1;
        stream += len1;
        audio_buf_index += len1;
    }
}

int FFmpegClass::packet_queue_put(PacketQueue *q, AVPacket *pkt) {
    AVPacketList *pkt1;
    if (av_packet_ref(pkt, pkt) < 0) {
        return -1;
    }
    pkt1 = (AVPacketList*)av_malloc(sizeof(AVPacketList));
    if (!pkt1) {
        return -1;
    }
    pkt1->pkt = *pkt;
    pkt1->next = NULL;
    
    
    SDL_LockMutex(q->mutex);
    
    if (!q->last_pkt) {
        q->first_pkt = pkt1;
    }
    else {
        q->last_pkt->next = pkt1;
    }
    q->last_pkt = pkt1;
    q->nb_packets++;
    q->size += pkt1->pkt.size;
    SDL_CondSignal(q->cond);
    
    SDL_UnlockMutex(q->mutex);
    return 0;
}

int FFmpegClass::packet_queue_get(PacketQueue *q, AVPacket *pkt, int block) {
    AVPacketList *pkt1;
    int ret;
    SDL_LockMutex(q->mutex);
    for (;;) {
        pkt1 = q->first_pkt;
        if (pkt1) {
            q->first_pkt = pkt1->next;
            if (!q->first_pkt) {
                q->last_pkt = NULL;
            }
            q->nb_packets--;
            q->size -= pkt1->pkt.size;
            *pkt = pkt1->pkt;
            av_free(pkt1);
            ret = 1;
            break;
        } else if (!block) {
            ret = 0;
            break;
        } else {
            SDL_CondWait(q->cond, q->mutex);
        }
    }
    SDL_UnlockMutex(q->mutex);
    return ret;
}
int FFmpegClass::audio_decode_frame(AVCodecContext *aCodecCtx, uint8_t *audio_buf, int buf_size) {
    static AVPacket pkt;
    static uint8_t *audio_pkt_data = NULL;
    static int audio_pkt_size = 0;
    static AVFrame frame;
    
    int len1, data_size = 0;
    for (;;) {
        while(audio_pkt_size > 0) {
            int got_frame = 0;
            len1 = avcodec_decode_audio4(aCodecCtx, &frame, &got_frame, &pkt);
            if (len1 < 0) {
                // if error, skip frame.
                audio_pkt_size = 0;
                break;
            }
            audio_pkt_data += len1;
            audio_pkt_size -= len1;
            if (got_frame) {
				//int dst_nb_samples = av_rescale_rnd(swr_get_delay(convert_ctx, frame.sample_rate) + frame.nb_samples, frame.sample_rate, frame.format, AVRounding(1));
				
                data_size = av_samples_get_buffer_size(NULL, aCodecCtx->channels, frame.nb_samples, aCodecCtx->sample_fmt, 0);
				swr_convert(convert_ctx, &audio_buf, data_size, (const uint8_t**)frame.data, frame.nb_samples);
				
				cprintf("In audio_decode_frame frame nb_samples is %d data_size %d\n", frame.nb_samples, data_size);
				data_size /= 2;
                //memcpy(audio_buf, frame.data[0], data_size);
				
            }
            if (data_size <= 0) {
                // No data yet, get more frames.
                continue;
            }
            // We have data, return it and come back for more later.
            return data_size;
        }
        if (pkt.data) {
            av_packet_unref(&pkt);
        }
        if (packet_queue_get(&audioq, &pkt, 1) < 0) {
            return -1;
        }
        audio_pkt_data = pkt.data;
        audio_pkt_size = pkt.size;
    }
}
int FFmpegClass::FFmpeg_openOutPutFile(char *path, char *type)
{
	frame_count = 0;
	ofmt = NULL;
    avformat_alloc_output_context2(&pFormatCtx_Out, ofmt, type, path);
    if(!pFormatCtx_Out){
        cprintf("create output context failed \r\n");
        return -1;
    }
    cprintf("create output context success \n");
	pFormatCtx_Out->video_codec_id =  AV_CODEC_ID_H264;
    ofmt = pFormatCtx_Out->oformat;
    video_st = NULL;
    if(ofmt->video_codec != AV_CODEC_ID_NONE){
		video_st = avformat_new_stream(pFormatCtx_Out, NULL);
		if(!video_st){
			cprintf("alloc video_st failed \n");
			return -3;
		}
		pCodec_Out = avcodec_find_encoder(AV_CODEC_ID_H264);
		if(pCodec_Out == NULL){
			cprintf("find encoder failed \n");
			return -4;
		}
		cprintf("find encoder success \n");
		AVStream *instream = pFormatCtx_In->streams[videoindex];
		video_st->codec->pix_fmt = instream->codec->pix_fmt;
		video_st->codec->width = pCodecCtxVideo_In->width;
		video_st->codec->height = pCodecCtxVideo_In->height;
		video_st->codec->time_base.den = 30;
		video_st->codec->time_base.num = 1;
		video_st->codec->gop_size = DEFAULT_FPS / 5;
		video_st->codec->bit_rate = 2 * 1024 * 1024;
		video_st->codec->codec_type = AVMEDIA_TYPE_VIDEO;
		video_st->codec->codec_tag = 0;
		video_st->codec->codec_id = AV_CODEC_ID_H264;
		video_st->codec->keyint_min = 30;
		video_st->codec->me_range = 16;
		video_st->codec->max_qdiff = 4;
		video_st->codec->qcompress = 0.6;
		video_st->codec->max_b_frames = 0;
		video_st->codec->b_frame_strategy = true;
		video_st->codec->qmin = 16;
		video_st->codec->qmax = 24;
		pCodecCtx_Out = video_st->codec;
		int n = CLOCK_TIME_BASE;
		av_opt_set(pCodecCtx_Out->priv_data, "preset", "superfast", 0);
	    av_opt_set(pCodecCtx_Out->priv_data, "tune", "zerolatency", 0);
		if(pFormatCtx_Out->oformat->flags&AVFMT_GLOBALHEADER){
			pCodecCtx_Out->flags |= CODEC_FLAG_GLOBAL_HEADER;
		}
		if (avcodec_open2(pCodecCtx_Out, pCodec_Out, NULL)<0) {
			cprintf("open encoder failed \n");
			return -5;
		}
		cprintf("open encoder success \n");
	}
    //av_dump_format(pFormatCtx_Out, 0, path, 1);
    if(!(ofmt->flags & AVFMT_NOFILE)){
        ret = avio_open(&pFormatCtx_Out->pb, path, AVIO_FLAG_WRITE);
        if(ret < 0){
            cprintf("Could not open output URL '%s'", path);
            return -2;
        }
    }
    printf("will write header \n");
    //д�ļ�ͷ
    ret = avformat_write_header(pFormatCtx_Out, NULL);
    if(ret < 0){
        printf("Error occurred when write header to output URL\n");
        return -6;
    }
    return 0;
}		

int FFmpegClass::Encoder_Init(){
	pCodec_Enc = avcodec_find_encoder(AV_CODEC_ID_H264);
	if(pCodec_Enc == NULL){
		cprintf("find encoder failed \n");
		return -4;
	}
	cprintf("find encoder success \n");
	pCodecCtx_Enc = avcodec_alloc_context3(pCodec_Enc);
	AVStream *instream = pFormatCtx_In->streams[videoindex];
	pCodecCtx_Enc->pix_fmt = instream->codec->pix_fmt;//���ظ�ʽ
	pCodecCtx_Enc->width = pCodecCtxVideo_In->width;
	pCodecCtx_Enc->height = pCodecCtxVideo_In->height;
	pCodecCtx_Enc->time_base.den = 30;//֡��
	pCodecCtx_Enc->time_base.num = 1;
	pCodecCtx_Enc->gop_size = 30;//I֡�������
	pCodecCtx_Enc->keyint_min = 30;//I֡��С�����
	pCodecCtx_Enc->bit_rate = 4*1024*1024;//Ŀ�����ʣ�����Խ����ƵԽ��
	pCodecCtx_Enc->rc_max_rate = instream->codec->rc_max_rate;
	pCodecCtx_Enc->codec_type = AVMEDIA_TYPE_VIDEO;//��������������
	pCodecCtx_Enc->codec_id = AV_CODEC_ID_H264;//������ID
	//pCodecCtx_Enc->me_range = 16;//�˶����뾶
	//pCodecCtx_Enc->max_qdiff = 4;//������֡��֮֡������б���������ӵı仯����
	//pCodecCtx_Enc->qcompress = 0.6;//������ѹ������0-1.ԽС�������Խ���ڹ̶���Խ��Խʹ�������������ڹ̶�
	pCodecCtx_Enc->max_b_frames = 0;//��ʹ��b֡
	//pCodecCtx_Enc->b_frame_strategy = false;//���Ϊtrue�����Զ�����ʲôʱ����Ҫ����B֡����ߴﵽ���õ����B֡�����������Ϊfalse,��ô����B֡����ʹ�á�
	//pCodecCtx_Enc->lmin = 1;//��С�������ճ��� 
	//pCodecCtx_Enc->lmax = 5;//����������ճ���
	//pCodecCtx_Enc->qmin = 10;//��С�������� ȡֵ��Χ1~51 һ��ȡֵ10~30
	//pCodecCtx_Enc->qmax = 50;//����������� ȡֵ��Χ1~51 һ��ȡֵ10~30
	//pCodecCtx_Enc->qblur = 0.0;//��ʾ�������������仯�̶ȣ�ȡֵ��Χ0.0~1.0��0.0��ʾ������
	//pCodecCtx_Enc->spatial_cplx_masking = 0.3;//�ռ临�Ӷ�
	//pCodecCtx_Enc->thread_count = 4;
	av_opt_set(pCodecCtx_Enc->priv_data, "preset", "superfast", 0);
	av_opt_set(pCodecCtx_Enc->priv_data, "tune", "zerolatency", 0);
	if (avcodec_open2(pCodecCtx_Enc, pCodec_Enc, NULL)<0){
		cprintf("open encoder failed \n");
		return -5;
	}
	cprintf("open encoder success \n");
	return 0;
}

int FFmpegClass::FFmeg_PlayPacket(AVPacket *packet){
	//if(av_read_frame(pFormatCtx_In, packet)>=0){//��һ֡����
	if(packet->stream_index == videoindex){
		ret = avcodec_decode_video2(pCodecCtxVideo_In, pFrameDec, &got_picture, packet);
		if(ret < 0){
			printf("�������\n");
			return -1;
		}
		if(got_picture){
			/*
			AVRational time_base = pFormatCtx_In->streams[videoindex]->time_base;
			AVRational time_base_q = {1, AV_TIME_BASE};
			int64_t pts_time = av_rescale_q(packet->dts, time_base, time_base_q);
			int64_t now_time = av_gettime() - start_time;
			if(pts_time > now_time){
			Sleep((pts_time - now_time)/1000.0);
			}*/
			sws_scale(img_convert_ctx, (const uint8_t* const*)pFrameDec->data, pFrameDec->linesize, 0, pCodecCtxVideo_In->height, pFrameYUV->data, pFrameYUV->linesize);
			//cvRectangle(out_buffer,p1 ,p2, CV_RGB(0, 255, 0), 2); //��ɫ����

			SDL_LockYUVOverlay(bmp);
			bmp->pixels[0]=pFrameYUV->data[0];
			bmp->pixels[2]=pFrameYUV->data[1];
			bmp->pixels[1]=pFrameYUV->data[2];
			bmp->pitches[0]=pFrameYUV->linesize[0];
			bmp->pitches[2]=pFrameYUV->linesize[1];
			bmp->pitches[1]=pFrameYUV->linesize[2];
			SDL_UnlockYUVOverlay(bmp);
			SDL_Rect *rect = (SDL_Rect*)malloc(sizeof(SDL_Rect));
			if (screen->clip_rect.w > pCodecCtxVideo_In->width) {
				rect->x = (screen->clip_rect.w - pCodecCtxVideo_In->width) / 2.0;
				rect->w = pCodecCtxVideo_In->width;
			}
			else {
				rect->x = 0;
				rect->w = screen->clip_rect.w;
			}
			if (screen->clip_rect.h > pCodecCtxVideo_In->height) {
				rect->y = (screen->clip_rect.h - pCodecCtxVideo_In->height) / 2.0;
				rect->h = pCodecCtxVideo_In->height;
			}
			else {
				rect->y = 0;
				rect->h = screen->clip_rect.h;
			}
			SDL_DisplayYUVOverlay(bmp, rect);
			free(rect);
		}
	}
	//}else{
	//	return -2;
	//}
	return 0;
}


int ANSIToUTF8(char* pszCode, char* UTF8code)
{
	WCHAR Unicode[100]={0,}; 
	char utf8[100]={0,};

	// read char Lenth
	int nUnicodeSize = MultiByteToWideChar(CP_ACP, 0, pszCode, strlen(pszCode), Unicode, sizeof(Unicode)); 
	memset(UTF8code, 0, nUnicodeSize+1);
	// read UTF-8 Lenth
	int nUTF8codeSize = WideCharToMultiByte(CP_UTF8, 0, Unicode, nUnicodeSize, UTF8code, sizeof(Unicode), NULL, NULL); 

	// convert to UTF-8 
	MultiByteToWideChar(CP_UTF8, 0, utf8, nUTF8codeSize, Unicode, sizeof(Unicode)); 
	UTF8code[nUTF8codeSize+1] = '\0';
	return nUTF8codeSize;
}
