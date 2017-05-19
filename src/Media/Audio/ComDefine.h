/*
 * ComDefine.h
 *
 *  Created on: Jan 5, 2016
 *      Author: 
 */

#ifndef COMDEFINE_H_
#define COMDEFINE_H_

#include <stdio.h>
#include <stdlib.h>

#include <android/log.h>

#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef TAG
#define TAG "ComDefine"
#endif

#define DEV_NAME	"/dev/video%d"


#define  LOGTAG false

#if LOGTAG
	#define ALOGV(...)  __android_log_print(ANDROID_LOG_VERBOSE,TAG,__VA_ARGS__)
	#define ALOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,	TAG,  __VA_ARGS__)
	#define ALOGI(...)  __android_log_print(ANDROID_LOG_INFO,	TAG,  __VA_ARGS__)
	#define ALOGW(...)  __android_log_print(ANDROID_LOG_INFO,	TAG,  __VA_ARGS__)
	#define ALOGE(...)  __android_log_print(ANDROID_LOG_ERROR,	TAG,  __VA_ARGS__)
#else
	#define ALOGV(...)  
	#define ALOGE(...)  
	#define ALOGD(...)  
	#define ALOGI(...)  
	#define ALOGW(...)  
#endif

#define AUDIOLOGW(...)  //__android_log_print(ANDROID_LOG_INFO,	TAG,  __VA_ARGS__)
#define TESTLOGW(...)	__android_log_print(ANDROID_LOG_INFO,	TAG,  __VA_ARGS__)

#ifndef SAFE_DELETE
#define SAFE_DELETE(p) 	do{delete(p); p = NULL;}while(false)
#endif

#define SAFE_DELETE_ELEMENT(ptr) if(ptr != NULL){ delete ptr; ptr = NULL;}

#ifndef SAFE_FREE
#define SAFE_FREE(p) 	do{free(p); p = NULL;}while(false)
#endif

enum	MACHINE_PATTERN
{
		  DOOR_MODE_FA = 1,
		  ROOM_MODE_FA
};


/*****************************************audio***********************************************/
const unsigned int AUDIO_INNER_CARD		= 0;
const unsigned int AUDIO_OUTER_CARD		= 1; //outer card
const unsigned int AUDIO_DEVICE			= 0;
const unsigned int AUDIO_CHANNELS		= 2;
const unsigned int AUDIO_SAMPLE_RATE	= 8000;
const unsigned int AUDIO_BITS			= 16;
const unsigned int AUDIO_PERIOD_SIZE	= 1024;
const unsigned int AUDIO_PERIOD_COUNT	= 4;
const unsigned int AUDIO_CAPTURE_TIME	= 0;



/*************************************************video*****************************************************/

enum	PIXEL_SCALE
{
	PIXEL_QCIF_176x144 = 0,
	PIXEL_QVGA_320x240,
	PIXEL_CIF_352x288,
	PIXEL_VGA_640x480,   //3
	PIXEL_SVGA_800x600,
	PIXEL_XGA_1024x768,
	PIXEL_HD720_1280x720, //6
	PIXEL_SXGA_1280x960,
	PIXEL_UXGA_1600x1200,
	PIXEL_HD1080_1920x1080, //9
	PIXEL_QXGA_2048x1536,
	PIXEL_QSXGA_2592x1936,
	PIXEL_PAL_720x576		//12
};	//total 12 items

#define VILLA_VIDEO_WIDTH		720
#define VILLA_VIDEO_HEIGHT		576

#define VIDEO_PIXEL	PIXEL_VGA_640x480

const int PixelArray[13][2] = { {176,144}, {320,240}, {352,288}, {640,480}, {800,600}, {1024,768}, {1280,720}, {1280,960}, {1600,1200}, {1920,1080}, {2048,1536}, {2592,1936}, {720,576} };

#define H264_BITRATE			3*1024*1024
#define VIDEO_FRAMERATE			30

#define CAMERA_NAME	"/dev/video%d"

#define  OPEN_VIDEO				true
#define  OPEN_AUDIO				true

#define LOCAL_CAMERA_RENDER		false	//直接回显摄像头，不编解码，不收发数据
#define CAMERA_CODEC_RENDER		false   //采集摄像头数据，编码，解码，回显
#define SENDER_SAVE_VIDEO		false	//保存编码好的视频数据
#define RECEIVE_SAVE_VIDEO		false	//保存接收到的视频数据
#define USE_AUDIO_CODEC			false	//需要音频编码
#define USE_AUDIO_CODEC_THREAD  false

const int CODEC_SLEEP_TIME		= 10*1000;//ms




/*******************************************************command callback************************************************************************/

enum	TALKBACK_EVENT
{
	START_RECV			= 0,
	STOP_RECV,
	START_TALKBACK,
	STOP_TALKBACK,
	VIDEO_INIT_SUCCES	= 10,
	VIDEO_INIT_FAILED,
	AUDIO_INIT_SUCCES	= 20,
	AUDIO_INIT_FAILED,
	RTP_INIT_SUCCES		= 30,
	RTP_INIT_FAILED,
	RTP_START_SEND_SUCCES,
	RTP_START_SEND_FAILED,
	RTP_STOP_SEND_SUCCES,
	RTP_STOP_SEND_FAILED,
	RTP_START_RECV_SUCCES,
	RTP_START_RECV_FAILED,
	RTP_STOP_RECV_SUCCES,
	RTP_STOP_RECV_FAILED//39
};

#define CALLBACK
typedef void (CALLBACK* RECV_CALLBACK)(void* pContext, int64_t timeStamp, char*mimeType, void* buffer, int dataLen);
typedef void (CALLBACK* COMMAND_CALLBACK)(void* pContext, char* cmd);


#define MIME_H264		"h264"
#define MIME_G711		"g711"
#define TALK_REQ		"talkreq"
#define TALK_SETUP		"setup"
#define TALK_TEAR	"talktear"
#define AUDIO_REQ		"audioreq"
#define AUDIO_TEAR		"audiotear"
#define VIDEO_REQ		"videoreq"
#define VIDEO_TEAR		"videotear"

const int MIME_H264_LEN		= strlen(MIME_H264)+1;
const int MIME_G711_LEN		= strlen(MIME_G711)+1;
const int TALK_REQ_LEN		= strlen(TALK_REQ)+1;
const int TALK_SETUP_LEN	= strlen(TALK_SETUP)+1;
const int TALK_TEAR_LEN = strlen(TALK_TEAR)+1;
const int AUDIO_REQ_LEN		= strlen(AUDIO_REQ)+1;
const int AUDIO_TEAR_LEN	= strlen(AUDIO_TEAR)+1;
const int VIDEO_REQ_LEN		= strlen(VIDEO_REQ)+1;
const int VIDEO_TEAR_LEN	= strlen(VIDEO_TEAR)+1;


#define MAX_PACKET_SIZE	((60* 1024) - 1)

/****************************************rtp***************************************************/

#define AUDIO_RECV_PORT  6000
#define AUDIO_SEND_PORT  7000

#define RECV_PORT  8000  //for rtp server bind
#define SEND_PORT  9000  //to connect the remote rtp receive server

/**
 *   当期设备时USB Camera 时，摄像头帧为YUYV420格式
 *   非ＵSB时，摄像头为NV12格式
 *
 */
#ifndef USB_CAMERA
#define USB_CAMERA 　　1
#endif

/**
 *  配置回显的同时是否进行
 *  Ａ20硬编码测试
 *
 */
#ifndef CONFIG_FATALKBACK_ENCODE_MOUDLE
#define CONFIG_FATALKBACK_ENCODE_MOUDLE 　　1
#endif

#endif /* COMDEFINE_H_ */
