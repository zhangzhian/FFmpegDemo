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
	unsigned char *rgb24_buffer = NULL;
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

	rgb24_buffer = (unsigned char *) malloc(width * height * 3);

	fwrite(pFrameRGB->data[0], width * height * 24 / 8, 1, fp_bmp);

	fclose(fp_bmp);
	free(rgb24_buffer);
	return 0;

}

/***
 * vai/mp4/h264转换成Bitmap
 * vai/mp4/h264->RGB->BMP
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