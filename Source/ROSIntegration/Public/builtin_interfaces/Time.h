#pragma once

#include "ROSBaseMsg.h"
#include "ROSTime.h"

namespace ROSMessages {
	namespace builtin_interfaces {
		class Time : public FROSBaseMsg {
		public:
			Time() : Time(FROSTime()) {}

			Time(FROSTime clock) 
			{
				_MessageType = "builtin_interfaces/Time";
				sec = clock._Sec;
				nanosec = clock._NSec;
			}

			// Seconds component, range is valid over any possible int32 value.
			int32 sec;

			// Nanoseconds component in the range of [0, 10e9)
			uint32 nanosec;
		};
	}
}
