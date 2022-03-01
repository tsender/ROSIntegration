#pragma once

#include "ROSBaseMsg.h"
#include "ROSTime.h"
#include "builtin_interfaces/Time.h"

namespace ROSMessages {
	namespace rosgraph_msgs {
		class Clock : public FROSBaseMsg {
		public:
			Clock() : Clock(FROSTime()) {}

			Clock(FROSTime in_clock) 
			{
				_MessageType = "rosgraph_msgs/Clock";
				clock = builtin_interfaces::Time(in_clock);
			}

			builtin_interfaces::Time clock;
		};
	}
}
