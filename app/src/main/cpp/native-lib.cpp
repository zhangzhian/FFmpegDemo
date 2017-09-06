#include <jni.h>
#include <string>
#include <stdio.h>
#include <time.h>

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavfilter/avfilter.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <android/log.h>
#include <libavutil/opt.h>
#include <libavutil/log.h>
#include <jni.h>
}

#ifdef ANDROID
#define LOGE(format, ...)  __android_log_print(ANDROID_LOG_ERROR, "(>_<)", format, ##__VA_ARGS__)
#define LOGI(format, ...)  __android_log_print(ANDROID_LOG_INFO,  "(^_^)", format, ##__VA_ARGS__)
#else
#define LOGE(format, ...)  printf("(>_<) " format "\n", ##__VA_ARGS__)
#define LOGI(format, ...)  printf("(^_^) " format "\n", ##__VA_ARGS__)
#endif

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55, 28, 1)
#define av_frame_alloc  avcodec_alloc_frame
#endif

#ifndef _WINGDI_
#define _WINGDI_
typedef unsigned long LONG;
typedef unsigned short WORD;
typedef unsigned long DWORD;
typedef struct tagBITMAPFILEHEADER {
	WORD bfType;
	DWORD bfSize;
	WORD bfReserved1;
	WORD bfReserved2;
	DWORD bfOffBits;
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER {
	DWORD biSize;
	LONG biWidth;
	LONG biHeight;
	WORD biPlanes;
	WORD biBitCount;
	DWORD biCompression;
	DWORD biSizeImage;
	LONG biXPelsPerMeter;
	LONG biYPelsPerMeter;
	DWORD biClrUsed;
	DWORD biClrImportant;
} BITMAPINFOHEADER;
#endif

//Output FFmpeg's av_log()
void custom_log(void *ptr, int level, const char *fmt, va_list vl) {
	FILE *fp = fopen("/storage/emulated/0/av_log.txt", "a+");
	if (fp) {
		vfprintf(fp, fmt, vl);
		fflush(fp);
		fclose(fp);
	}
}

/**
 * 单个RGB->BMP
 * @param rgb24path
 * @param width
 * @param height
 * @param bmppath
 * @return
 */

int simplest_rgb24_to_bmp(const char *rgb24path, int width, int height, const char *bmppath) {
	typedef struct {
		long imageSize;
		long blank;
		long startPosition;
	} BmpHead;

	typedef struct {
		long Length;
		long width;
		long height;
		unsigned short colorPlane;
		unsigned short bitColor;
		long zipFormat;
		long realSize;
		long xPels;
		long yPels;
		long colorUse;
		long colorImportant;
	} InfoHead;

	int i = 0, j = 0;
	BmpHead m_BMPHeader = {0};
	InfoHead m_BMPInfoHeader = {0};
	char bfType[2] = {'B', 'M'};
	int header_size = sizeof(bfType) + sizeof(BmpHead) + sizeof(InfoHead);
	unsigned char *rgb24_buffer = NULL;
	FILE *fp_rgb24 = NULL, *fp_bmp = NULL;

	if ((fp_rgb24 = fopen(rgb24path, "rb")) == NULL) {
		printf("Error: Cannot open input RGB24 file.\n");
		return -1;
	}
	if ((fp_bmp = fopen(bmppath, "wb")) == NULL) {
		printf("Error: Cannot open output BMP file.\n");
		return -1;
	}

	rgb24_buffer = (unsigned char *) malloc(width * height * 3);
	fread(rgb24_buffer, 1, width * height * 3, fp_rgb24);

	m_BMPHeader.imageSize = 3 * width * height + header_size;
	m_BMPHeader.startPosition = header_size;

	m_BMPInfoHeader.Length = sizeof(InfoHead);
	m_BMPInfoHeader.width = width;
	//BMP storage pixel data in opposite direction of Y-axis (from bottom to top).
	m_BMPInfoHeader.height = -height;
	m_BMPInfoHeader.colorPlane = 1;
	m_BMPInfoHeader.bitColor = 24;
	m_BMPInfoHeader.realSize = 3 * width * height;

	fwrite(bfType, 1, sizeof(bfType), fp_bmp);
	fwrite(&m_BMPHeader, 1, sizeof(m_BMPHeader), fp_bmp);
	fwrite(&m_BMPInfoHeader, 1, sizeof(m_BMPInfoHeader), fp_bmp);
	//BMP save R1|G1|B1,R2|G2|B2 as B1|G1|R1,B2|G2|R2
	//It saves pixel data in Little Endian
	//So we change 'R' and 'B'
	for (j = 0; j < height; j++) {
		for (i = 0; i < width; i++) {
			char temp = rgb24_buffer[(j * width + i) * 3 + 2];
			rgb24_buffer[(j * width + i) * 3 + 2] = rgb24_buffer[(j * width + i) * 3 + 0];
			rgb24_buffer[(j * width + i) * 3 + 0] = temp;
		}
	}
	fwrite(rgb24_buffer, 3 * width * height, 1, fp_bmp);

	fclose(fp_rgb24);
	fclose(fp_bmp);
	free(rgb24_buffer);
	printf("Finish generate %s!\n", bmppath);
	return 0;
}

/**
 * RGB数据->BMP
 * @param rgb24_buffer
 * @param width
 * @param height
 * @param index
 * @return
 */
int SaveAsBMP(uint8_t *rgb24_buffer, int width, int height, int index) {
	typedef struct {
		long imageSize;
		long blank;
		long startPosition;
	} BmpHead;

	typedef struct {
		long Length;
		long width;
		long height;
		unsigned short colorPlane;
		unsigned short bitColor;
		long zipFormat;
		long realSize;
		long xPels;
		long yPels;
		long colorUse;
		long colorImportant;
	} InfoHead;

	int i = 0, j = 0;
	BmpHead m_BMPHeader = {0};
	InfoHead m_BMPInfoHeader = {0};
	char bfType[2] = {'B', 'M'};
	int header_size = sizeof(bfType) + sizeof(BmpHead) + sizeof(InfoHead);
//	unsigned char *rgb24_buffer = NULL;
	FILE *fp_bmp = NULL;

	char *filename = new char[255];  //文件存放路径，根据自己的修改
	sprintf(filename, "%s_%d.bmp", "/storage/emulated/0/Download/avtest/img/", index);

//	if ((fp_rgb24 = fopen(rgb24path, "rb")) == NULL) {
//		printf("Error: Cannot open input RGB24 file.\n");
//		return -1;
//	}
	if ((fp_bmp = fopen(filename, "wb")) == NULL) {
		printf("Error: Cannot open output BMP file.\n");
		return -1;
	}

//	rgb24_buffer = (unsigned char *) malloc(width * height * 3);
//	fread(rgb24_buffer, 1, width * height * 3, str);
	m_BMPHeader.imageSize = 3 * width * height + header_size;
	m_BMPHeader.startPosition = header_size;

	m_BMPInfoHeader.Length = sizeof(InfoHead);
	m_BMPInfoHeader.width = width;
	//BMP storage pixel data in opposite direction of Y-axis (from bottom to top).
	m_BMPInfoHeader.height = -height;
	m_BMPInfoHeader.colorPlane = 1;
	m_BMPInfoHeader.bitColor = 24;
	m_BMPInfoHeader.realSize = 3 * width * height;

	fwrite(bfType, 1, sizeof(bfType), fp_bmp);
	fwrite(&m_BMPHeader, 1, sizeof(m_BMPHeader), fp_bmp);
	fwrite(&m_BMPInfoHeader, 1, sizeof(m_BMPInfoHeader), fp_bmp);
	//BMP save R1|G1|B1,R2|G2|B2 as B1|G1|R1,B2|G2|R2
	//It saves pixel data in Little Endian
	//So we change 'R' and 'B'
	for (j = 0; j < height; j++) {
		for (i = 0; i < width; i++) {
			char temp = rgb24_buffer[(j * width + i) * 3 + 2];
			rgb24_buffer[(j * width + i) * 3 + 2] = rgb24_buffer[(j * width + i) * 3 + 0];
			rgb24_buffer[(j * width + i) * 3 + 0] = temp;
		}
	}

	fwrite(rgb24_buffer, 3 * width * height, 1, fp_bmp);
	fclose(fp_bmp);
	return 0;
}


/**
 * RGB帧->BMP
 * 保存BMP文件的函数
 * @param pFrameRGB
 * @param width
 * @param height
 * @param index
 * @return
 */

int SaveAsBMP(AVFrame *pFrameRGB, int width, int height, int index) {
	typedef struct {
		long imageSize;
		long blank;
		long startPosition;
	} BmpHead;

	typedef struct {
		long Length;
		long width;
		long height;
		unsigned short colorPlane;
		unsigned short bitColor;
		long zipFormat;
		long realSize;
		long xPels;
		long yPels;
		long colorUse;
		long colorImportant;
	} InfoHead;
	char *filename = new char[255];  //文件存放路径，根据自己的修改

	sprintf(filename, "%s_%d.bmp", "/storage/emulated/0/Download/avtest/img/", index);

	BmpHead m_BMPHeader = {0};
	InfoHead m_BMPInfoHeader = {0};
	char bfType[2] = {'B', 'M'};
	int header_size = sizeof(bfType) + sizeof(BmpHead) + sizeof(InfoHead);
	FILE *fp_bmp = NULL;
	if ((fp_bmp = fopen(filename, "wb")) == NULL) {
		printf("Error: Cannot open output BMP file.\n");
		return -1;
	}
	m_BMPHeader.imageSize = 3 * width * height + header_size;
	m_BMPHeader.startPosition = header_size;

	m_BMPInfoHeader.Length = sizeof(InfoHead);
	m_BMPInfoHeader.width = width;
	//BMP storage pixel data in opposite direction of Y-axis (from bottom to top).
	m_BMPInfoHeader.height = -height;
	m_BMPInfoHeader.colorPlane = 1;
	m_BMPInfoHeader.bitColor = 24;
	m_BMPInfoHeader.realSize = 3 * width * height;

	fwrite(bfType, 1, sizeof(bfType), fp_bmp);
	fwrite(&m_BMPHeader, 1, sizeof(m_BMPHeader), fp_bmp);
	fwrite(&m_BMPInfoHeader, 1, sizeof(m_BMPInfoHeader), fp_bmp);


	fwrite(pFrameRGB->data[0], width * height * 24 / 8, 1, fp_bmp);
	fclose(fp_bmp);
	return 0;

}

/***
 * avi/mp4/h264转换成Bitmap
 * avi/mp4/h264->RGB->BMP
 */
extern "C"
JNIEXPORT jint JNICALL
Java_com_yodosmart_ffmpegdemo_MainActivity_avToBitmap(JNIEnv *env, jobject instance,
													  jstring input_jstr, jstring output_jstr) {
	//h264ToYue
	AVFormatContext *pFormatCtx;
	int i, videoindex;
	AVCodecContext *pCodecCtx;
	AVCodec *pCodec;
	AVFrame *pFrame, *pFrameRGB;
	uint8_t *out_buffer;
	AVPacket *packet;
	int y_size;
	int ret, got_picture;
	struct SwsContext *img_convert_ctx;
	struct SwsContext *img_convert_ctx_rgb;
	FILE *fp_yuv;
	int frame_cnt;
	clock_t time_start, time_finish;
	double time_duration = 0.0;

	char input_str[500] = {0};
	char output_str[500] = {0};
	char info[1000] = {0};
	sprintf(input_str, "%s", env->GetStringUTFChars(input_jstr, NULL));
	sprintf(output_str, "%s", env->GetStringUTFChars(output_jstr, NULL));
	FILE *output = fopen(output_str, "wb+");
	//FFmpeg av_log() callback
	av_log_set_callback(custom_log);

	av_register_all();
	avformat_network_init();
	pFormatCtx = avformat_alloc_context();

	if (avformat_open_input(&pFormatCtx, input_str, NULL, NULL) != 0) {
		LOGE("Couldn't open input stream.\n");
		return -1;
	}
	if (avformat_find_stream_info(pFormatCtx, NULL) < 0) {
		LOGE("Couldn't find stream information.\n");
		return -1;
	}
	videoindex = -1;
	for (i = 0; i < pFormatCtx->nb_streams; i++)
		if (pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoindex = i;
			break;
		}
	if (videoindex == -1) {
		LOGE("Couldn't find a video stream.\n");
		return -1;
	}
	pCodecCtx = pFormatCtx->streams[videoindex]->codec;
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if (pCodec == NULL) {
		LOGE("Couldn't find Codec.\n");
		return -1;
	}
	if (avcodec_open2(pCodecCtx, pCodec, NULL) < 0) {
		LOGE("Couldn't open codec.\n");
		return -1;
	}

	pFrame = av_frame_alloc();
	pFrameRGB = av_frame_alloc();
	//	out_buffer = (unsigned char *) av_malloc(
	//			av_image_get_buffer_size(AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1));
	//	av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, out_buffer,
	//		 AV_PIX_FMT_YUV420P, pCodecCtx->width, pCodecCtx->height, 1);
	out_buffer = (unsigned char *) av_malloc(
			av_image_get_buffer_size(AV_PIX_FMT_BGR24, pCodecCtx->width, pCodecCtx->height, 1));
	av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, out_buffer,
						 AV_PIX_FMT_BGR24, pCodecCtx->width, pCodecCtx->height, 1);


	packet = (AVPacket *) av_malloc(sizeof(AVPacket));

	//	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
	//	 pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_YUV420P,
	//	 SWS_BICUBIC, NULL, NULL, NULL);
	img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height, pCodecCtx->pix_fmt,
									 pCodecCtx->width, pCodecCtx->height, AV_PIX_FMT_BGR24,
									 SWS_BICUBIC, NULL, NULL, NULL);

	sprintf(info, "[Input     ]%s\n", input_str);
	sprintf(info, "%s[Output    ]%s\n", info, output_str);
	sprintf(info, "%s[Format    ]%s\n", info, pFormatCtx->iformat->name);
	sprintf(info, "%s[Codec     ]%s\n", info, pCodecCtx->codec->name);
	sprintf(info, "%s[Resolution]%dx%d\n", info, pCodecCtx->width, pCodecCtx->height);


	fp_yuv = fopen(output_str, "wb+");
	if (fp_yuv == NULL) {
		printf("Cannot open output file.\n");
		return -1;
	}

	frame_cnt = 0;
	time_start = clock();

	while (av_read_frame(pFormatCtx, packet) >= 0) {
		if (packet->stream_index == videoindex) {
			ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
			if (ret < 0) {
				LOGE("Decode Error.\n");
				return -1;
			}
			if (got_picture) {
				sws_scale(img_convert_ctx, (const uint8_t *const *) pFrame->data, pFrame->linesize,
						  0, pCodecCtx->height,
						  pFrameRGB->data, pFrameRGB->linesize);
				y_size = pCodecCtx->width * pCodecCtx->height;

				//RGB
				//转换
				fwrite(pFrameRGB->data[0], (pCodecCtx->width) * (pCodecCtx->height) * 3, 1, output);
				SaveAsBMP(pFrameRGB, pCodecCtx->width, pCodecCtx->height, frame_cnt);
				//fwrite(pFrameRGB->data[0], 1, y_size, fp_yuv);    //Y
				//fwrite(pFrameRGB->data[1], 1, y_size / 4, fp_yuv);  //U
				//fwrite(pFrameRGB->data[2], 1, y_size / 4, fp_yuv);  //V
				//Output info
				char pictype_str[10] = {0};
				switch (pFrame->pict_type) {
					case AV_PICTURE_TYPE_I:
						sprintf(pictype_str, "I");
						break;
					case AV_PICTURE_TYPE_P:
						sprintf(pictype_str, "P");
						break;
					case AV_PICTURE_TYPE_B:
						sprintf(pictype_str, "B");
						break;
					default:
						sprintf(pictype_str, "Other");
						break;
				}
				LOGI("Frame Index: %5d. Type:%s", frame_cnt, pictype_str);
				frame_cnt++;
			}
		}
		av_free_packet(packet);
	}

	//flush decoder
	//FIX: Flush Frames remained in Codec
	while (1) {
		ret = avcodec_decode_video2(pCodecCtx, pFrame, &got_picture, packet);
		if (ret < 0)
			break;
		if (!got_picture)
			break;
		//	sws_scale(img_convert_ctx, (const uint8_t *const *) pFrame->data, pFrame->linesize, 0,
		//  pCodecCtx->height,
		//  pFrameRGB->data, pFrameRGB->linesize);
		//	int y_size = pCodecCtx->width * pCodecCtx->height;

		sws_scale(img_convert_ctx, (const uint8_t *const *) pFrame->data, pFrame->linesize, 0,
				  pCodecCtx->height,
				  pFrameRGB->data, pFrameRGB->linesize);
		int y_size = pCodecCtx->width * pCodecCtx->height;
		//RGB
		//转换
		fwrite(pFrameRGB->data[0], (pCodecCtx->width) * (pCodecCtx->height) * 3, 1, output);


		SaveAsBMP(pFrameRGB, pCodecCtx->width, pCodecCtx->height, frame_cnt);
		//		fwrite(pFrameRGB->data[0], 1, y_size, fp_yuv);    //Y
		//		fwrite(pFrameRGB->data[1], 1, y_size / 4, fp_yuv);  //U
		//		fwrite(pFrameRGB->data[2], 1, y_size / 4, fp_yuv);  //V
		//Output info
		char pictype_str[10] = {0};
		switch (pFrame->pict_type) {
			case AV_PICTURE_TYPE_I:
				sprintf(pictype_str, "I");
				break;
			case AV_PICTURE_TYPE_P:
				sprintf(pictype_str, "P");
				break;
			case AV_PICTURE_TYPE_B:
				sprintf(pictype_str, "B");
				break;
			default:
				sprintf(pictype_str, "Other");
				break;
		}
		LOGI("Frame Index: %5d. Type:%s", frame_cnt, pictype_str);
		frame_cnt++;
	}
	time_finish = clock();
	time_duration = (double) (time_finish - time_start);

	sprintf(info, "%s[Time      ]%fms\n", info, time_duration);
	sprintf(info, "%s[Count     ]%d\n", info, frame_cnt);

	sws_freeContext(img_convert_ctx);

	fclose(fp_yuv);

	av_frame_free(&pFrameRGB);
	av_frame_free(&pFrame);
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);
	//simplest_rgb24_to_bmp("lena_256x256_rgb24.rgb", 256, 256, "output_lena.bmp");
	return frame_cnt;
}
using namespace std;

int flush_encoder(AVFormatContext *fmt_ctx, unsigned int stream_index);

/***
 * yuv转换成mp4
 */

extern "C"
JNIEXPORT jint JNICALL
Java_com_yodosmart_ffmpegdemo_MainActivity_yuvToMp4(JNIEnv *env, jobject instance,
													jstring input_jstr_, jstring output_jstr_,
													jint w_jstr_, jint h_jstr_, jint num_jstr_) {
	const char *input_jstr = env->GetStringUTFChars(input_jstr_, 0);
	const char *output_jstr = env->GetStringUTFChars(output_jstr_, 0);
	AVFormatContext *pFormatCtx;
	AVOutputFormat *fmt;
	AVStream *video_st;
	AVCodecContext *pCodecCtx;
	AVCodec *pCodec;
	AVPacket pkt;
	uint8_t *picture_buf;
	AVFrame *pFrame;
	int picture_size;
	int y_size;
	int framecnt = 0;
	//FILE *in_file = fopen("src01_480x272.yuv", "rb"); //Input raw YUV data
	FILE *in_file = fopen(input_jstr, "rb");   //Input raw YUV data



	int in_w = w_jstr_, in_h = h_jstr_;                              //Input data's width and height
	int framenum = num_jstr_;                                   //Frames to encode
	//const char* out_file = "src01.h264";              //Output Filepath
	//const char* out_file = "src01.ts";
	//const char* out_file = "src01.hevc";
	const char *out_file = output_jstr;

	av_register_all();
	//Method1.
	pFormatCtx = avformat_alloc_context();
	//Guess Format
	fmt = av_guess_format(NULL, out_file, NULL);
	pFormatCtx->oformat = fmt;

	//Method 2.
	//avformat_alloc_output_context2(&pFormatCtx, NULL, NULL, out_file);
	//fmt = pFormatCtx->oformat;


	//Open output URL
	if (avio_open(&pFormatCtx->pb, out_file, AVIO_FLAG_READ_WRITE) < 0) {
		printf("Failed to open output file! \n");
		return -1;
	}

	video_st = avformat_new_stream(pFormatCtx, 0);
	//video_st->time_base.num = 1;
	//video_st->time_base.den = 25;

	if (video_st == NULL) {
		return -1;
	}
	//Param that must set
	pCodecCtx = video_st->codec;
	//pCodecCtx->codec_id =AV_CODEC_ID_HEVC;
	pCodecCtx->codec_id = fmt->video_codec;
	pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
	pCodecCtx->pix_fmt = AV_PIX_FMT_YUV420P;
	pCodecCtx->width = in_w;
	pCodecCtx->height = in_h;
	pCodecCtx->bit_rate = 400000;
	pCodecCtx->gop_size = 250;

	pCodecCtx->time_base.num = 1;
	pCodecCtx->time_base.den = 25;

	//H264
	//pCodecCtx->me_range = 16;
	//pCodecCtx->max_qdiff = 4;
	//pCodecCtx->qcompress = 0.6;
	pCodecCtx->qmin = 10;
	pCodecCtx->qmax = 51;

	//Optional Param
	pCodecCtx->max_b_frames = 3;

	// Set Option
	AVDictionary *param = 0;
	//H.264
	if (pCodecCtx->codec_id == AV_CODEC_ID_H264) {
		av_dict_set(&param, "preset", "slow", 0);
		av_dict_set(&param, "tune", "zerolatency", 0);
		//av_dict_set(¶m, "profile", "main", 0);
	}
	//H.265
	if (pCodecCtx->codec_id == AV_CODEC_ID_H265) {
		av_dict_set(&param, "preset", "ultrafast", 0);
		av_dict_set(&param, "tune", "zero-latency", 0);
	}

	//Show some Information
	av_dump_format(pFormatCtx, 0, out_file, 1);

	pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
	if (!pCodec) {
		printf("Can not find encoder! \n");
		return -1;
	}
	if (avcodec_open2(pCodecCtx, pCodec, &param) < 0) {
		printf("Failed to open encoder! \n");
		return -1;
	}


	pFrame = av_frame_alloc();
	picture_size = avpicture_get_size(pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height);
	picture_buf = (uint8_t *) av_malloc(picture_size);
	avpicture_fill((AVPicture *) pFrame, picture_buf, pCodecCtx->pix_fmt, pCodecCtx->width,
				   pCodecCtx->height);

	//Write File Header
	avformat_write_header(pFormatCtx, NULL);

	av_new_packet(&pkt, picture_size);

	y_size = pCodecCtx->width * pCodecCtx->height;

	for (int i = 0; i < framenum; i++) {
		//Read raw YUV data
		if (fread(picture_buf, 1, y_size * 3 / 2, in_file) <= 0) {
			printf("Failed to read raw data! \n");
			return -1;
		} else if (feof(in_file)) {
			break;
		}
		pFrame->data[0] = picture_buf;              // Y
		pFrame->data[1] = picture_buf + y_size;      // U
		pFrame->data[2] = picture_buf + y_size * 5 / 4;  // V
		//PTS
		//pFrame->pts=i;
		pFrame->pts = i * (video_st->time_base.den) / ((video_st->time_base.num) * 25);
		int got_picture = 0;
		//Encode
		int ret = avcodec_encode_video2(pCodecCtx, &pkt, pFrame, &got_picture);
		if (ret < 0) {
			printf("Failed to encode! \n");
			return -1;
		}
		if (got_picture == 1) {
			printf("Succeed to encode frame: %5d\tsize:%5d\n", framecnt, pkt.size);
			framecnt++;
			pkt.stream_index = video_st->index;
			ret = av_write_frame(pFormatCtx, &pkt);
			av_free_packet(&pkt);
		}
	}
	//Flush Encoder
	int ret = flush_encoder(pFormatCtx, 0);
	if (ret < 0) {
		printf("Flushing encoder failed\n");
		return -1;
	}

	//Write file trailer
	av_write_trailer(pFormatCtx);

	//Clean
	if (video_st) {
		avcodec_close(video_st->codec);
		av_free(pFrame);
		av_free(picture_buf);
	}
	avio_close(pFormatCtx->pb);
	avformat_free_context(pFormatCtx);

	fclose(in_file);

	return 0;

}

int flush_encoder(AVFormatContext *fmt_ctx, unsigned int stream_index) {
	int ret;
	int got_frame;
	AVPacket enc_pkt;
	if (!(fmt_ctx->streams[stream_index]->codec->codec->capabilities &
		  CODEC_CAP_DELAY))
		return 0;
	while (1) {
		enc_pkt.data = NULL;
		enc_pkt.size = 0;
		av_init_packet(&enc_pkt);
		ret = avcodec_encode_video2(fmt_ctx->streams[stream_index]->codec, &enc_pkt,
									NULL, &got_frame);
		av_frame_free(NULL);
		if (ret < 0)
			break;
		if (!got_frame) {
			ret = 0;
			break;
		}
		printf("Flush Encoder: Succeed to encode 1 frame!\tsize:%5d\n", enc_pkt.size);
		/* mux encoded frame */
		ret = av_write_frame(fmt_ctx, &enc_pkt);
		if (ret < 0)
			break;
	}
	return ret;
}

/***
 * yuv转换成bmp
 */
extern "C"
JNIEXPORT jint JNICALL
Java_com_yodosmart_ffmpegdemo_MainActivity_yuvToBitmap(JNIEnv *env, jobject instance,
													   jstring input_jstr_, jstring output_jstr_,
													   jint w_jstr, jint h_jstr) {
	const char *input_jstr = env->GetStringUTFChars(input_jstr_, 0);
	const char *output_jstr = env->GetStringUTFChars(output_jstr_, 0);

	//Parameters
	FILE *src_file = fopen(input_jstr, "rb");
	const int src_w = w_jstr, src_h = h_jstr;
	AVPixelFormat src_pixfmt = AV_PIX_FMT_YUV420P;

	int src_bpp = av_get_bits_per_pixel(av_pix_fmt_desc_get(src_pixfmt));

	FILE *dst_file = fopen(output_jstr, "wb");
	const int dst_w = w_jstr, dst_h = h_jstr;
	AVPixelFormat dst_pixfmt = AV_PIX_FMT_RGB24;
	int dst_bpp = av_get_bits_per_pixel(av_pix_fmt_desc_get(dst_pixfmt));

	//Structures
	uint8_t *src_data[4];
	int src_linesize[4];

	uint8_t *dst_data[4];
	int dst_linesize[4];

	int rescale_method = SWS_BICUBIC;
	struct SwsContext *img_convert_ctx;
	uint8_t *temp_buffer = (uint8_t *) malloc(src_w * src_h * src_bpp / 8);

	int frame_idx = 0;
	int ret = 0;
	ret = av_image_alloc(src_data, src_linesize, src_w, src_h, src_pixfmt, 1);
	if (ret < 0) {
		printf("Could not allocate source image\n");
		return -1;
	}
	ret = av_image_alloc(dst_data, dst_linesize, dst_w, dst_h, dst_pixfmt, 1);
	if (ret < 0) {
		printf("Could not allocate destination image\n");
		return -1;
	}
	//-----------------------------
	//Init Method 1
	img_convert_ctx = sws_alloc_context();
	//Show AVOption
//	av_opt_show2(img_convert_ctx, stdout, AV_OPT_FLAG_VIDEO_PARAM, 0);
	//Set Value
//	av_opt_set_int(img_convert_ctx, "sws_flags", SWS_BICUBIC | SWS_PRINT_INFO, 0);
	av_opt_set_int(img_convert_ctx, "srcw", src_w, 0);
	av_opt_set_int(img_convert_ctx, "srch", src_h, 0);
	av_opt_set_int(img_convert_ctx, "src_format", src_pixfmt, 0);
	//'0' for MPEG (Y:0-235);'1' for JPEG (Y:0-255)
	av_opt_set_int(img_convert_ctx, "src_range", 1, 0);
	av_opt_set_int(img_convert_ctx, "dstw", dst_w, 0);
	av_opt_set_int(img_convert_ctx, "dsth", dst_h, 0);
	av_opt_set_int(img_convert_ctx, "dst_format", dst_pixfmt, 0);
	av_opt_set_int(img_convert_ctx, "dst_range", 1, 0);
	sws_init_context(img_convert_ctx, NULL, NULL);

	//Init Method 2
	//img_convert_ctx = sws_getContext(src_w, src_h,src_pixfmt, dst_w, dst_h, dst_pixfmt,
	//  rescale_method, NULL, NULL, NULL);
	//-----------------------------
	/*
	//Colorspace
	ret=sws_setColorspaceDetails(img_convert_ctx,sws_getCoefficients(SWS_CS_ITU601),0,
		sws_getCoefficients(SWS_CS_ITU709),0,
		 0, 1 << 16, 1 << 16);
	if (ret==-1) {
		printf( "Colorspace not support.\n");
		return -1;
	}
	*/
	while (1) {
		if (fread(temp_buffer, 1, src_w * src_h * src_bpp / 8, src_file) !=
			src_w * src_h * src_bpp / 8) {
			break;
		}

		switch (src_pixfmt) {
			case AV_PIX_FMT_GRAY8: {
				memcpy(src_data[0], temp_buffer, src_w * src_h);
				break;
			}
			case AV_PIX_FMT_YUV420P: {
				memcpy(src_data[0], temp_buffer, src_w * src_h);                    //Y
				memcpy(src_data[1], temp_buffer + src_w * src_h, src_w * src_h / 4);      //U
				memcpy(src_data[2], temp_buffer + src_w * src_h * 5 / 4, src_w * src_h / 4);  //V
				break;
			}
			case AV_PIX_FMT_YUV422P: {
				memcpy(src_data[0], temp_buffer, src_w * src_h);                    //Y
				memcpy(src_data[1], temp_buffer + src_w * src_h, src_w * src_h / 2);      //U
				memcpy(src_data[2], temp_buffer + src_w * src_h * 3 / 2, src_w * src_h / 2);  //V
				break;
			}
			case AV_PIX_FMT_YUV444P: {
				memcpy(src_data[0], temp_buffer, src_w * src_h);                    //Y
				memcpy(src_data[1], temp_buffer + src_w * src_h, src_w * src_h);        //U
				memcpy(src_data[2], temp_buffer + src_w * src_h * 2, src_w * src_h);      //V
				break;
			}
			case AV_PIX_FMT_YUYV422: {
				memcpy(src_data[0], temp_buffer, src_w * src_h * 2);                  //Packed
				break;
			}
			case AV_PIX_FMT_RGB24: {
				memcpy(src_data[0], temp_buffer, src_w * src_h * 3);                  //Packed
				break;
			}
			default: {
				printf("Not Support Input Pixel Format.\n");
				break;
			}
		}

		sws_scale(img_convert_ctx, (const uint8_t *const *) src_data, src_linesize, 0, src_h,
				  dst_data, dst_linesize);
		printf("Finish process frame %5d\n", frame_idx);

		switch (dst_pixfmt) {
			case AV_PIX_FMT_GRAY8: {
				fwrite(dst_data[0], 1, dst_w * dst_h, dst_file);
				break;
			}
			case AV_PIX_FMT_YUV420P: {
				fwrite(dst_data[0], 1, dst_w * dst_h, dst_file);                 //Y
				fwrite(dst_data[1], 1, dst_w * dst_h / 4, dst_file);               //U
				fwrite(dst_data[2], 1, dst_w * dst_h / 4, dst_file);               //V
				break;
			}
			case AV_PIX_FMT_YUV422P: {
				fwrite(dst_data[0], 1, dst_w * dst_h, dst_file);                 //Y
				fwrite(dst_data[1], 1, dst_w * dst_h / 2, dst_file);               //U
				fwrite(dst_data[2], 1, dst_w * dst_h / 2, dst_file);               //V
				break;
			}
			case AV_PIX_FMT_YUV444P: {
				fwrite(dst_data[0], 1, dst_w * dst_h, dst_file);                 //Y
				fwrite(dst_data[1], 1, dst_w * dst_h, dst_file);                 //U
				fwrite(dst_data[2], 1, dst_w * dst_h, dst_file);                 //V
				break;
			}
			case AV_PIX_FMT_YUYV422: {
				fwrite(dst_data[0], 1, dst_w * dst_h * 2, dst_file);               //Packed
				break;
			}
			case AV_PIX_FMT_RGB24: {
				fwrite(dst_data[0], 1, dst_w * dst_h * 3, dst_file);               //Packed
				SaveAsBMP(dst_data[0], src_w, src_h, frame_idx);
				break;
			}
			case AV_PIX_FMT_BGR24: {
				fwrite(dst_data[0], 1, dst_w * dst_h * 3, dst_file);               //Packed
				SaveAsBMP(dst_data[0], src_w, src_h, frame_idx);
				break;
			}
			default: {
				printf("Not Support Output Pixel Format.\n");
				break;
			}
		}

		frame_idx++;
	}

	sws_freeContext(img_convert_ctx);

	free(temp_buffer);
	fclose(dst_file);
	av_freep(&src_data[0]);
	av_freep(&dst_data[0]);


	return frame_idx;
}


extern "C"
JNIEXPORT jint JNICALL
Java_com_yodosmart_ffmpegdemo_MainActivity_RGBToBitmap(JNIEnv *env, jobject instance) {

	return simplest_rgb24_to_bmp("/storage/emulated/0/out.rgb", 640, 360,
								 "/storage/emulated/0/out.bmp");

}



extern "C"
JNIEXPORT jstring JNICALL
Java_com_yodosmart_ffmpegdemo_MainActivity_stringFromJNI(
		JNIEnv *env,
		jobject /* this */) {
	std::string hello = "Hello from C++";
	return env->NewStringUTF(hello.c_str());
}


extern "C"
JNIEXPORT jstring JNICALL
Java_com_yodosmart_ffmpegdemo_MainActivity_urlprotocolinfo(
		JNIEnv *env, jobject) {
	char info[40000] = {0};
	av_register_all();

	struct URLProtocol *pup = NULL;

	struct URLProtocol **p_temp = &pup;
	avio_enum_protocols((void **) p_temp, 0);

	while ((*p_temp) != NULL) {
		sprintf(info, "%sInput: %s\n", info, avio_enum_protocols((void **) p_temp, 0));
	}
	pup = NULL;
	avio_enum_protocols((void **) p_temp, 1);
	while ((*p_temp) != NULL) {
		sprintf(info, "%sInput: %s\n", info, avio_enum_protocols((void **) p_temp, 1));
	}
	return env->NewStringUTF(info);
}
extern "C"
JNIEXPORT jstring JNICALL
Java_com_yodosmart_ffmpegdemo_MainActivity_avformatinfo(
		JNIEnv *env, jobject) {
	char info[40000] = {0};

	av_register_all();

	AVInputFormat *if_temp = av_iformat_next(NULL);
	AVOutputFormat *of_temp = av_oformat_next(NULL);
	while (if_temp != NULL) {
		sprintf(info, "%sInput: %s\n", info, if_temp->name);
		if_temp = if_temp->next;
	}
	while (of_temp != NULL) {
		sprintf(info, "%sOutput: %s\n", info, of_temp->name);
		of_temp = of_temp->next;
	}
	return env->NewStringUTF(info);
}
extern "C"
JNIEXPORT jstring JNICALL
Java_com_yodosmart_ffmpegdemo_MainActivity_avcodecinfo(
		JNIEnv *env, jobject) {
	char info[40000] = {0};

	av_register_all();

	AVCodec *c_temp = av_codec_next(NULL);

	while (c_temp != NULL) {
		if (c_temp->decode != NULL) {
			sprintf(info, "%sdecode:", info);
		} else {
			sprintf(info, "%sencode:", info);
		}
		switch (c_temp->type) {
			case AVMEDIA_TYPE_VIDEO:
				sprintf(info, "%s(video):", info);
				break;
			case AVMEDIA_TYPE_AUDIO:
				sprintf(info, "%s(audio):", info);
				break;
			default:
				sprintf(info, "%s(other):", info);
				break;
		}
		sprintf(info, "%s[%10s]\n", info, c_temp->name);
		c_temp = c_temp->next;
	}

	return env->NewStringUTF(info);
}
extern "C"
JNIEXPORT jstring JNICALL
Java_com_yodosmart_ffmpegdemo_MainActivity_avfilterinfo(
		JNIEnv *env, jobject) {
	char info[40000] = {0};
	avfilter_register_all();

	AVFilter *f_temp = (AVFilter *) avfilter_next(NULL);
	while (f_temp != NULL) {
		sprintf(info, "%s%s\n", info, f_temp->name);
		f_temp = f_temp->next;
	}
	return env->NewStringUTF(info);
}