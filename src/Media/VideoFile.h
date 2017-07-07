

#ifndef VIDEO_FILE_H_
#define VIDEO_FILE_H_

#include <utils/threads.h>


#include "VideoBase.h"
#include "CodecBase.h"

using namespace android;


class VideoFile : public VideoBase
{
	public:
		VideoFile();
		virtual ~VideoFile();

		bool Init(sp<AMessage> &format, sp<Surface> &surface, sp<ICrypto> &crypto, int flags);
		bool DeInit();
		bool IsInited();
		
		bool StartVideo(int cardid);
		bool StopVideo();
		void RegisterRecorder(RecordBase*record);

		void ReceiveSource(int64_t timeStamp, char*mimeType, void* buffer, int dataLen);
		
		void GetFileSource();

    class FileProcessThread:public Thread 
    {
		public:
			FileProcessThread(VideoFile *file) :
						  Thread(false), mFileSource(file) {
			}

			void startThread() {
				run("FileProcessThread", ANDROID_PRIORITY_URGENT_AUDIO);
			}
			
			void stopThread() {
				requestExit();
			}
			virtual bool threadLoop() 
			{
				// loop until we need to quit
				mFileSource->GetFileSource();
				return true;
			}
	
		private:
			VideoFile* mFileSource;
    };

    sp<FileProcessThread> mThread;
		
	protected:
		bool threadLoop();
		bool setBuffer(void* data, int len, int64_t timeStamp);
		bool isFirstFrame();
		void setFirstFrame(bool bFirst);

	private:
		bool mbInited;
		bool					mRunning;
		FILE 					*mFile;
		char* 					mFileBuffer;
		int 					mStartIndex;
		int 					mFileLen;

	    sp<CodecBase>  		mCodec;

		
		
};


#endif



