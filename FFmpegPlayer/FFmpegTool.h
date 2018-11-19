#pragma once
 extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
	//新版里的图像转换结构需要引入的头文件
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/time.h"
#include "libavutil/imgutils.h"
#include "libavutil/opt.h"
#include "sdl/SDL.h"
#include "sdl/SDL_thread.h"
}; 

#define DEFAULT_FPS 30
#define VIDEO_BIT_RATE 10 * 1024 * 1024
#define MAX_AUDIO_FRAME_SIZE 192000
struct EncoderStruct{
	AVCodecContext  *pCodecCtx;
	int used;
};
typedef struct PacketQueue {
    AVPacketList *first_pkt, *last_pkt;
    int nb_packets;
    int size;
    SDL_mutex *mutex;
    SDL_cond *cond;
} PacketQueue;


class FFmpegClass
{
public:
	FFmpegClass();
	void FFmpeg_Init();
	int Encoder_Init();
	int FFmpeg_openFile(char *fileName);
	int FFmpeg_openOutPutFile(char *path, char *type);
	
	int FFmeg_PlayPacket(AVPacket *packet);

	PacketQueue audioq;
	void packet_queue_init(PacketQueue *q);
	int packet_queue_put(PacketQueue *q, AVPacket *pkt);
	int packet_queue_get(PacketQueue *q, AVPacket *pkt, int block);
	int audio_decode_frame(AVCodecContext *aCodecCtx, uint8_t *audio_buf, int buf_size);
	
	
	AVFrame *pFrameDec;
	AVFrame *pFrameYUV;
	AVFrame *pFramePCM;
	AVFrame *pFrameEnc;
	struct SwrContext *convert_ctx ;
	AVFormatContext	*pFormatCtx_In;
	AVFormatContext	*pFormatCtx_Out;
	AVCodecContext  *pCodecCtxVideo_In;
	AVCodecContext  *pCodecCtxAudio_In;
	AVCodecContext  *pCodecCtx_Out;
	AVCodecContext  *pCodecCtx_Enc;
	SDL_AudioSpec wanted_spec, spec;
	AVCodec         *pCodecVideo_In;
	AVCodec         *pCodecAudio_In;
	AVCodec         *pCodec_Out;
	AVCodec         *pCodec_Enc;
	AVOutputFormat *ofmt;
	AVStream *video_st;
	
	AVPicture src_picture, dst_picture;
	
	SDL_Surface *screen;
	SDL_Overlay *bmp;
	
	int ret;
	int pcm_size;
	int y_size;
    int got_picture;
	int got_packet;
	int64_t start_time;
	struct SwsContext *img_convert_ctx;
	
	uint8_t *out_buffer;
	int videoindex;
	int audioindex;
};

#define STREAM_DURATION   5.0  
#define STREAM_FRAME_RATE 25 /* 25 images/s */  
#define STREAM_NB_FRAMES  ((int)(STREAM_DURATION * STREAM_FRAME_RATE))  
int ANSIToUTF8(char* pszCode, char* UTF8code);
void audio_callback(void *userdata, Uint8 *stream, int len);

