#ifndef CAMERA_API_H
#define CAMERA_API_H


#include "IVideoCallback.h"

using namespace android;

#ifdef __cplusplus
extern "C"
{
#endif


	int CameraSetup(IVideoCallback *callback, jint cameraId);
	int CameraRelease();
	void SetCameraParameter(String8 params8);
	String8 GetCameraParameter();
	void StartPreview(const sp<Surface> &surface);
	void StopPreview();

#ifdef __cplusplus
}
#endif


#endif