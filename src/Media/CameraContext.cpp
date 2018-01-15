#include "CameraContext.h"

#include<dlfcn.h>

#include <utils/Log.h>



#define TAG "CameraLib"

namespace android 
{

	CameraLib* CameraLib::mSelf = NULL;

	CameraLib::CameraLib():mLibHandle(NULL)
	{
		
	}

	CameraLib::~CameraLib()
	{
		if(mLibHandle)
		{
			dlclose(mLibHandle); // 关闭so句柄
			mLibHandle = NULL;
		}
		
	}

	CameraLib*CameraLib::getInstance()
	{
		if(mSelf == NULL)
			mSelf = new CameraLib();
		return mSelf;
	}

	bool CameraLib::LoadCameraLib(int version)
	{
		if(mLibHandle)
			return false;
		
			switch(version)
			{
				case 17: 		//17 4.2, 4.2.2
				case 18: 		//18 4.3
				case 19:		//19 4.4
				case 20:		//20 4.4W
				mLibHandle = dlopen("libCamera4.so", RTLD_NOW); // 载入.so文件 RTLD_LAZY
					break;
					
				case 21: 		//21 5.0
				case 22:		//22 5.1
				mLibHandle = dlopen("libCamera5.so", RTLD_NOW); // 载入.so文件 RTLD_LAZY
					break;
					
				case 23: 		//6.0
				mLibHandle = dlopen("libCamera6.so", RTLD_NOW); // 载入.so文件 RTLD_LAZY
					break;
					
				case 24:		//7.0
				case 25:		//7.1.1/7.1
					break;
					
				case 26:		//8.0 O
					break;
				
				default:
					break;
			}
			
			if (!mLibHandle) 
			{
				ALOGE("Error: load so  failed.\n");
			}
			dlerror(); 			//清空错误信息
		
		
		CameraSetup 					= (CameraSetup_t*)dlsym(mLibHandle, "CameraSetup");
		CameraRelease 					= (CameraRelease_t*)dlsym(mLibHandle, "CameraRelease");
		SetCameraParameter 				= (SetCameraParameter_t*)dlsym(mLibHandle, "SetCameraParameter");
		GetCameraParameter 				= (GetCameraParameter_t*)dlsym(mLibHandle, "GetCameraParameter");
		StartPreview 					= (StartPreview_t*)dlsym(mLibHandle, "StartPreview");
		StopPreview 					= (StopPreview_t*)dlsym(mLibHandle, "StopPreview");
		SetDisplayOrientation			= (SetDisplayOrientation_t*)dlsym(mLibHandle, "SetDisplayOrientation");
		
		
		const char *err = dlerror();
		if (NULL != err) 
		{
			ALOGE("dlsym stderr:%s\n", err);
			return false;
		}
			
		return mLibHandle!=NULL;
	}

}


