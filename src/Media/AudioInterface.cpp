/*
 * 
 *
 *  Created on: 
 *      Author: 
 */

#include "jni.h"
#include <android/native_window_jni.h>
#include <android/log.h>
#include <stdio.h>
#include <unistd.h>



#define ALOGV(...)  __android_log_print(ANDROID_LOG_VERBOSE,TAG,__VA_ARGS__)
#define ALOGE(...)  __android_log_print(ANDROID_LOG_ERROR,	TAG,  __VA_ARGS__)
#define ALOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,	TAG,  __VA_ARGS__)
#define ALOGI(...)  __android_log_print(ANDROID_LOG_INFO,	TAG,  __VA_ARGS__)
#define ALOGW(...)  __android_log_print(ANDROID_LOG_INFO,	TAG,  __VA_ARGS__)


#include "AudioInterface.h"

#define TAG "AudioInterface" 

#ifdef __cplusplus
extern "C" 
{
#endif

#ifndef NULL
#define NULL ((void *)0)
#endif

#ifndef NELEM
# define NELEM(x) ((int) (sizeof(x) / sizeof((x)[0])))
#endif

/**
 *   JAVA类属性/函数映射JNIID
 *
 */

struct fields_t 
{
	/**
	 * JAVA类属性
	 */
	jfieldID    context;
	/***
	 * JAVA类函数
	 */
	jmethodID   post_event;
	jmethodID   rect_constructor;
	jobject		mjobj;
	JNIEnv*		menv;
};

static struct fields_t fields;

/**
 * 当在ANativeWindow_Buffer绘图时，需将
 * 摄像头YUV420 ==>WINDOW_FORMAT_RGB_565
 *
 *
 */
static ANativeWindow_Buffer* buffer=NULL;

/**
 *   类属性，描述结构表
 *
 *
 */
struct field 
{
	const char *class_name; // java 类名 :"android/hardware/Camera$CameraInfo"
	const char *field_name; // 类属性名：  orientation
	const char *field_type; // 类属性类型： "I"
	jfieldID   *jfield;     //　对应熟悉fieldID
};


JavaVM*		g_javaVM = NULL;

 
 
 
/**
 *
 * 查找某个java 类
 *
 *注意　env->FindClass
 *注意　env->GetFieldID
 *注意　　Ｃ，Ｃ++　的不同
 *　　　　　　　Ｃ：  　(*env)->FindClass（env....
 *　　　　　　　Ｃ++： env->FindClass（....
 *　　　　　　参考：JNI line 502
 *
 */
static int find_fields(JNIEnv *env, struct field *fields, int count)
{
	for (int i = 0; i < count; i++) 
	{
		struct field *f = &fields[i];
		jclass clazz = env->FindClass(f->class_name);
		if (clazz == NULL) 
		{
			ALOGE("Can't find %s", f->class_name);
			return -1;
		}

		jfieldID field = env->GetFieldID(clazz,f->field_name,f->field_type);
		if (field == NULL) 
		{
			ALOGE("Can't find %s.%s", f->class_name, f->field_name);
			return -1;
		}
		*(f->jfield) = field;
	}
	return 0;
}


/**
 *
 *  向JVM　函数映射表中注册JAVA类的Ｎative函数
 *  编译器内置调试变量(f:%s, l:%d)
 *	　　__FUNCTION__ 　函数名
 *	　　__LINE__　　　　　　当期行号
 */
/*
int jniRegisterNativeMethods(JNIEnv* env, const char* className, const JNINativeMethod* gMethods, int numMethods)
{
	ALOGV("Enter:jniRegisterNativeMethods----------->");
	ALOGV("Registering %s natives", className);
	jclass clazz = env->FindClass(className);
	if(clazz == NULL)
	{
		ALOGE("Do not Find Class %s,(f:%s, l:%d)", className,__FUNCTION__, __LINE__);
		return -1;
	}
	if (env->RegisterNatives(clazz, gMethods, numMethods) < 0) 
	{
		ALOGE("RegisterNatives failed for %s, aborting", className);
		return -1;
	}
	ALOGV("Leave:jniRegisterNativeMethods<-----------");
	return JNI_TRUE;
}
*/

/***
 *
 *  VM 加载依赖的so文件时，会调用本函数
 *
 *　动态显示注册jni函数
 *　并初始化java相关类的熟悉：jfieldID
 *　						：MethodID
 *　						.........
 *　	　　app 中唯一的	JVM　对象
 *
 */
 /*
int JNI_OnLoad(JavaVM* vm, void* reserved)
{
	//JNIEnv* env = NULL;
	jint result = -1;
	g_javaVM = vm;
	return JNI_VERSION_1_4; 
}
*/

#ifdef __cplusplus
}
#endif


