#pragma once

#include "ROSBaseMsg.h"
#include "ROSTime.h"
#include "builtin_interfaces/Time.h"

namespace ROSMessages{
	namespace std_msgs {
		class Header: public FROSBaseMsg {
		public:
			Header() 
			{
				_MessageType = "std_msgs/Header";
			}

			Header(FROSTime Stamp, FString FrameID)  
			{
				_MessageType = "std_msgs/Header";
				stamp = builtin_interfaces::Time(Stamp);
				frame_id = FrameID;
			}

			builtin_interfaces::Time stamp;
			FString frame_id;
		};
	}
}
