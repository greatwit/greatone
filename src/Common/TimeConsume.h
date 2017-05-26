#ifndef TIME_CONSUME_H
#define TIME_CONSUME_H
 

#define	 TAG "TimeConsume"
#include "ComDefine.h"

#include <sys/time.h> 
#include <utils/threads.h>


	class TimeConsume
	{
		public:	
			TimeConsume();
			virtual ~TimeConsume();

			void			TimeBegin();
			void			TimeEnd();
			int				TimeCountMicroSecond(char*tag);
			int				TimeCountMilliSecond (char*tag);

		public:
		static TimeConsume* GetInstance();

		private:
			static TimeConsume* mpInstance;
			struct timeval mBeginTime;
			struct timeval mEndTime;
	};



#endif // CAMERA_HARDWARE_H
