#ifndef _CAMERA_CONTEXT_H_
#define _CAMERA_CONTEXT_H_

#include <stdlib.h>

#include <gui/Surface.h>
#include <media/ICrypto.h>
#include <media/stagefright/foundation/AMessage.h>
#include "android_runtime/AndroidRuntime.h"

#include "IVideoCallback.h"

using namespace android;

typedef int CameraSetup_t(IVideoCallback *callback, jint cameraId);
typedef int CameraRelease_t();
typedef void SetCameraParameter_t(String8 params8);
typedef String8 GetCameraParameter_t();
typedef void StartPreview_t(const sp<Surface> &surface);
typedef void StopPreview_t();

namespace android 
{
	

class CameraLib
{
	public:
		CameraLib();
		~CameraLib();
		static CameraLib*getInstance();
		bool LoadCameraLib(int version);
		
		CameraSetup_t* 				CameraSetup;
		CameraRelease_t*			CameraRelease;
		SetCameraParameter_t*		SetCameraParameter;
		GetCameraParameter_t* 		GetCameraParameter;
		StartPreview_t*				StartPreview;
		StopPreview_t*				StopPreview;

		
	private:
		static CameraLib*mSelf;
		void *mLibHandle;
};

}

#endif