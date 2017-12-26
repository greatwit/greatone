
#include "FileEnCodec.h"
#include "RtpReceive.h"


#include "ComDefine.h"
#define TAG "FileEnCodec"


#define ALOGTEST(...)	__android_log_print(ANDROID_LOG_INFO,	TAG,  __VA_ARGS__)
#define ALOGE(...)		__android_log_print(ANDROID_LOG_ERROR,	TAG,  __VA_ARGS__)


int intToBytes(int val , char*outData) 
{   
	outData[0] = val;
	outData[1] = val >> 8;     
	outData[2] = val >> 16;
	outData[3] = val >> 24;
				
	return 4;  
} 

FileEnCodec::FileEnCodec()
			:mbRunning(false)
			,mFile(NULL)
{
	ALOGV("SenderServer::SenderServer() construct.");
	mSpsPpsLen = 0;
}

FileEnCodec::~FileEnCodec()
{
	ALOGV("SenderServer, Destructor");
}



bool FileEnCodec::CreateCodec( const sp<AMessage> &format, const sp<Surface> &surface, const sp<ICrypto> &crypto, int flags, char*filename)
{
	mcharLength[4] = {0};
	mData[1000000] = {0};

	mFile = fopen(filename, "wb");
	
	//mCodec = new CodecBase("video/avc", true, true);
	//mCodec->CreateCodec(format, surface, crypto, flags);
	//mCodec->registerBufferCall(this);
	CodecBaseLib::getInstance()->CodecCreate(format, surface, crypto, flags, true);
	CodecBaseLib::getInstance()->RegisterBufferCall(this);
	
	return true;
}

bool FileEnCodec::DeInit()
{	
	if(mbRunning)
		StopVideo();
	
	fclose(mFile);
	
	return true;
}


bool FileEnCodec::StartVideo(int deivceid)
{

	//mCodec->startCodec();
	CodecBaseLib::getInstance()->StartCodec();

	VIDEOLOGD("TAG 2,function %s,line:%d",__FUNCTION__,__LINE__);


	return true;
}

bool FileEnCodec::StopVideo()
{

	ALOGW("TAG 1,function %s,line:%d StopVideo 0",__FUNCTION__,__LINE__);

	//mCodec->stopCodec();
	CodecBaseLib::getInstance()->StopCodec();
	
	VIDEOLOGD("TAG 1,function %s,line:%d StopVideo 2",__FUNCTION__,__LINE__);

	return true;
}

void FileEnCodec::onCodecBuffer(struct CodecBuffer& buff)
{
	ALOGTEST("onCodecBuffer--size:%d flags:%d", buff.size, buff.flags);
	int size = buff.size;
	char lenBuf[4] = {0};
	intToBytes(size, lenBuf);
	fwrite(lenBuf, 4, 1, mFile);
	fwrite(buff.buf, size, 1, mFile);
}

void FileEnCodec::AddDecodecSource(char *data, int len)
{
	//mCodec->addBuffer(data, len);
	CodecBaseLib::getInstance()->AddBuffer(data, len);
}



