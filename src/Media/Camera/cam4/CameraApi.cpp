

#include "jni.h"
#include "JNIHelp.h"
#include "android_runtime/AndroidRuntime.h"

#include <cutils/properties.h>
#include <utils/Vector.h>

#include <gui/SurfaceTexture.h>
#include <gui/Surface.h>
#include <camera/Camera.h>
#include <binder/IMemory.h>


#include "CameraApi.h"
#define TAG "CameraApi"

using namespace android;

// provides persistent context for calls from native code to Java
class JNICameraListen: public CameraListener
{
	public:
		JNICameraListen();
		~JNICameraListen() { }
		sp<Camera> getCamera();
		
		virtual void notify(int32_t msgType, int32_t ext1, int32_t ext2);
		virtual void postData(int32_t msgType, const sp<IMemory>& dataPtr, camera_frame_metadata_t *metadata);
		virtual void postDataTimestamp(nsecs_t timestamp, int32_t msgType, const sp<IMemory>& dataPtr);
		
		
		// connect to camera service
		int CameraSetup(jobject thiz, jint cameraId);
		// disconnect from camera service
		// It's okay to call this when the native camera context is already null.
		// This handles the case where the user has called release() and the
		// finalizer is invoked later.
		int CameraRelease();

		void SetCameraParameter(String8 params8);
		String8 GetCameraParameter();
		void StartPreview(const sp<Surface> &surface);
		void StopPreview();

	private:
		sp<Camera>  mCamera;                // strong reference to native object

};

JNICameraListen::JNICameraListen():mCamera(NULL)
{

}

sp<Camera> JNICameraListen::getCamera()
{
	return mCamera;
}

void JNICameraListen::notify(int32_t msgType, int32_t ext1, int32_t ext2)
{
    ALOGV("notify");
}

void JNICameraListen::postData(int32_t msgType, const sp<IMemory>& dataPtr, camera_frame_metadata_t *metadata)
{
	/*
    // VM pointer will be NULL if object is released
    JNIEnv *env = AndroidRuntime::getJNIEnv();


    int32_t dataMsgType = msgType & ~CAMERA_MSG_PREVIEW_METADATA;

    // return data based on callback type
    switch (dataMsgType) {
        case CAMERA_MSG_VIDEO_FRAME:
            // should never happen
            break;

        // For backward-compatibility purpose, if there is no callback
        // buffer for raw image, the callback returns null.
        case CAMERA_MSG_RAW_IMAGE:
            break;

        // There is no data.
        case 0:
            break;

        default:
            ALOGV("dataCallback(%d, %p)", dataMsgType, dataPtr.get());
            break;
    }

    // post frame metadata to Java
    if (metadata && (msgType & CAMERA_MSG_PREVIEW_METADATA)) {
        //postMetadata(env, CAMERA_MSG_PREVIEW_METADATA, metadata);
    }
	*/
	if (dataPtr != NULL) {
		ssize_t offset;
		size_t size;
		sp<IMemoryHeap> heap = dataPtr->getMemory(&offset, &size);
		
		uint8_t *heapBase = (uint8_t*)heap->base();
		if (heapBase != NULL)
		{
			const jbyte* data = reinterpret_cast<const jbyte*>(heapBase + offset);
			//CodecBaseLib::getInstance()->AddBuffer((char*)data, size);
			free((void*)data);
			data = NULL;
		}
		
		ALOGE("postData: off=%ld, size=%d", offset, size);
		mCamera->setPreviewCallbackFlags(CAMERA_FRAME_CALLBACK_FLAG_BARCODE_SCANNER);
	}
}

void JNICameraListen::postDataTimestamp(nsecs_t timestamp, int32_t msgType, const sp<IMemory>& dataPtr)
{
    // TODO: plumb up to Java. For now, just drop the timestamp
}



// connect to camera service
int JNICameraListen::CameraSetup(jobject thiz, jint cameraId)
{
	if(mCamera == NULL)
		mCamera = Camera::connect(cameraId);

    if (mCamera == NULL) {
        ALOGV("Fail to connect to camera service");
        return -1;
    }

    // make sure camera hardware is alive
    if (mCamera->getStatus() != NO_ERROR) {
        ALOGV("Camera initialization failed");
        return -2;
    }

	// We use a weak reference so the Camera object can be garbage collected.
    // The reference is only used as a proxy for callbacks.
	//if(gListenContext == NULL)
	//	gListenContext = new JNICameraListen(camera);

    mCamera->setListener(this);
	mCamera->setPreviewCallbackFlags(CAMERA_FRAME_CALLBACK_FLAG_BARCODE_SCANNER);

	return 0;
}

// disconnect from camera service
// It's okay to call this when the native camera context is already null.
// This handles the case where the user has called release() and the
// finalizer is invoked later.
int JNICameraListen::CameraRelease()
{
    // TODO: Change to ALOGV
    ALOGV("release camera");

	// clear callbacks
	if (mCamera != NULL) {
		mCamera->setPreviewCallbackFlags(CAMERA_FRAME_CALLBACK_FLAG_NOOP);
		mCamera->disconnect();
		mCamera.clear();
		mCamera = NULL;
	}

	// remove context to prevent further Java access
	//context->decStrong(thiz);
	
	return 0;
}

void JNICameraListen::SetCameraParameter(String8 params8)
{
	if (mCamera->setParameters(params8) != NO_ERROR) {
		ALOGE("Camera setParameters failed");
	}
}

String8 JNICameraListen::GetCameraParameter()
{
	return mCamera->getParameters();
}

void JNICameraListen::StartPreview(const sp<Surface> &surface)
{
    ALOGV("startPreview");

	if(surface!=NULL && mCamera != NULL)
		if (mCamera->setPreviewDisplay(surface) != NO_ERROR)
			ALOGE("Camera setPreviewDisplay failed");
	
    if (mCamera->startPreview() != NO_ERROR)
        ALOGE( "startPreview failed");
}

void JNICameraListen::StopPreview()
{
    ALOGV("stopPreview");

    mCamera->stopPreview();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
sp<JNICameraListen> gListenContext = NULL;

int CameraSetup(jobject thiz, jint cameraId)
{
	if(gListenContext == NULL)
		gListenContext = new JNICameraListen();
	
	return gListenContext->CameraSetup(thiz, cameraId);
}

int CameraRelease()
{
	if(gListenContext != NULL)
	{
		gListenContext->CameraRelease();
		gListenContext = NULL;
		return 0;
	}
		
	return -1;
}

void SetCameraParameter(String8 params8)
{
	if(gListenContext != NULL)
		gListenContext->SetCameraParameter(params8);
}

String8 GetCameraParameter()
{
	return gListenContext->GetCameraParameter();
}

void StartPreview(const sp<Surface> &surface)
{
	if(gListenContext != NULL)
		gListenContext->StartPreview(surface);
}

void StopPreview()
{
	if(gListenContext != NULL)
		gListenContext->StopPreview();
}
