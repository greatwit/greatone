#ifndef CODEC_SENDER_H_
#define CODEC_SENDER_H_

#include <utils/threads.h> 
#include <utils/Log.h>

#include "RtpSender.h"
#include <string>
#include <map>

#include "Common.h"

#include "android_runtime/AndroidRuntime.h"
#include "android_runtime/android_view_Surface.h"
#include <media/ICrypto.h>
#include <media/stagefright/MediaCodec.h>
#include <media/stagefright/foundation/ABuffer.h>
#include <media/stagefright/foundation/ADebug.h>
#include <media/stagefright/foundation/ALooper.h>
#include <media/stagefright/foundation/AMessage.h>
#include <media/stagefright/foundation/AString.h>
#include <media/stagefright/MediaErrors.h>

#include "CameraHardware.h" 
#include "IVideoCallback.h"

#include <gui/Surface.h> 

#include <camera/Camera.h>
#include <binder/IMemory.h>

using namespace android;

class CodecSender : public CameraListener, public ICodecCallback
{
	public:
		CodecSender();
		virtual ~CodecSender();
		bool CreateCodec(JNIEnv *env, jobject thiz, const sp<AMessage> &format, const sp<Surface> &surface, const sp<ICrypto> &crypto, int flags, short sendPort);
		bool CreateCodec(jobject thiz, const sp<AMessage> &format, const sp<Surface> &surface, const sp<ICrypto> &crypto, int flags, short sendPort, int cameraId);
		
		bool DeInit();
		
		void SetCameraParameter(String8 params8);
		String8 GetCameraParameter();
		
		bool ConnectDest(std::string ip, short port);
		bool StartVideo();
		bool StopVideo();

		void AddDecodecSource(char *data, int len);
		virtual void onCodecBuffer(struct CodecBuffer& buff);
		
		virtual void notify(int32_t msgType, int32_t ext1, int32_t ext2);
		virtual void postData(int32_t msgType, const sp<IMemory>& dataPtr, camera_frame_metadata_t *metadata);
		virtual void postDataTimestamp(nsecs_t timestamp, int32_t msgType, const sp<IMemory>& dataPtr);
		
	protected:
		class CameraContext : public CameraListener
		{
			public:
				CameraContext(CodecSender *codec):mCodec(codec){};
				~CameraContext(){};
				virtual void notify(int32_t msgType, int32_t ext1, int32_t ext2){}
				virtual void postData(int32_t msgType, const sp<IMemory>& dataPtr, camera_frame_metadata_t *metadata)
				{
					if (dataPtr != NULL) {
						ssize_t offset;
						size_t size;
						sp<IMemoryHeap> heap = dataPtr->getMemory(&offset, &size);
						
						//uint8_t *heapBase = (uint8_t*)heap->base();
						//if (heapBase != NULL)
						{
							//const jbyte* data = reinterpret_cast<const jbyte*>(heapBase + offset);
							//CodecBaseLib::getInstance()->AddBuffer((char*)heapBase, size);
						}
						
						ALOGE("CameraContext: off=%ld, size=%d", offset, size);
					}
				}
				virtual void postDataTimestamp(nsecs_t timestamp, int32_t msgType, const sp<IMemory>& dataPtr)
				{
					
				}
				
			private:
				CodecSender *mCodec;
		};

	private:
		bool				mFirstFrame;
		bool				mbRunning;
		sp<Camera> 			mCamera;
		sp<CameraContext>   mContext;
		//sp<CodecBase> 	mCodec;
		RtpSender					*mpSender;
};

#endif


