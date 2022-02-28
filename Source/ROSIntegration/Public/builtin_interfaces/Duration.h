#pragma once

#include "ROSBaseMsg.h"

namespace ROSMessages {
	namespace builtin_interfaces {
		class Duration : public FROSBaseMsg {
		public:
			Duration() : Duration(0,0) {}

			Duration(int32 InSec, uint32 InNanosec) 
			{
				_MessageType = "builtin_interfaces/Duration";
				sec = InSec;
				nanosec = InNanosec;
			}

			// Seconds component, range is valid over any possible int32 value.
			int32 sec;

			// Nanoseconds component in the range of [0, 10e9)
			uint32 nanosec;
		};
	}
}
