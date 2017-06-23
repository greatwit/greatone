/*
 * Copyright 2012, The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _ANDROID_MEDIA_MEDIACODEC_H_
#define _ANDROID_MEDIA_MEDIACODEC_H_

#include "jni.h"

#include <media/hardware/CryptoAPI.h>
#include <media/stagefright/foundation/ABase.h>
#include <utils/Errors.h>
#include <utils/RefBase.h>

#include <gui/Surface.h>
#include <gui/SurfaceTextureClient.h>

#include <media/ICrypto.h>

#define ALOGTEST(...)	__android_log_print(ANDROID_LOG_INFO,	TAG,  __VA_ARGS__)
#define ALOGE(...)		__android_log_print(ANDROID_LOG_ERROR,	TAG,  __VA_ARGS__)

namespace android {

struct ALooper;
struct AMessage;
struct AString;
struct ICrypto;
struct ISurfaceTexture;
struct MediaCodec;
struct SurfaceTextureClient;

struct GMediaCodec : public RefBase 
{
    GMediaCodec( const char *name, bool nameIsType, bool encoder);

    status_t initCheck() const;

    status_t configure(const sp<AMessage> &format, const sp<ISurfaceTexture> &surfaceTexture, const sp<ICrypto> &crypto, int flags);

    status_t start();
    status_t stop();

    status_t flush();

    status_t queueInputBuffer( size_t index, size_t offset, size_t size, int64_t timeUs, uint32_t flags, AString *errorDetailMsg);

    status_t dequeueInputBuffer(size_t *index, int64_t timeoutUs);

    status_t dequeueOutputBuffer( jobject bufferInfo, size_t *index, int64_t timeoutUs);
	status_t dequeueOutputBuffer( size_t *index, int64_t timeoutUs);

    status_t releaseOutputBuffer(size_t index, bool render);

    status_t getOutputFormat(jobject *format) const;

	status_t getBuffers(  bool input, jobjectArray *bufArray);
	
	status_t addBuffer(char*data, int len);
	
	status_t startCodec();
	status_t stopCodec();

protected:
    virtual ~GMediaCodec();
	

private:
    jclass mClass;
    jweak mObject;
    sp<SurfaceTextureClient> mSurfaceTextureClient;

    sp<ALooper> 	mLooper;
    sp<MediaCodec> 	mCodec;
	
	uint8_t*		mpBufferPoint[2];
	bool			mbRunning;
	
	pthread_t		mPid;
	pthread_mutex_t mMutex;
	pthread_cond_t	mCond;        
	JNIEnv 			*mpEnv;

    DISALLOW_EVIL_CONSTRUCTORS(GMediaCodec);
};

}  // namespace android

#endif  // _ANDROID_MEDIA_MEDIACODEC_H_
