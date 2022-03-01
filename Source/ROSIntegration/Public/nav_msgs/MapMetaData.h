//
//  MapMetaData.h
//  HFS
//
//  Created by Timothy Saucer on 2/19/19.
//  Copyright © 2019 Epic Games, Inc. All rights reserved.
//

#pragma once

#include "ROSBaseMsg.h"
#include "geometry_msgs/Pose.h"
#include "builtin_interfaces/Time.h"

namespace ROSMessages {
	namespace nav_msgs {
		class MapMetaData : public FROSBaseMsg {
		public:
			MapMetaData() {
				_MessageType = "nav_msgs/MapMetaData";
			}

			MapMetaData(FROSTime in_map_load_time, float in_resolution, uint32 in_width, uint32 in_height, geometry_msgs::Pose in_origin)
			{
				_MessageType = "nav_msgs/MapMetaData";
				map_load_time = builtin_interfaces::Time(in_map_load_time);
				resolution = in_resolution;
				width = in_width;
				height = in_height;
				origin = in_origin;
			}

			// time map_load_time
			builtin_interfaces::Time map_load_time;

			//float32 resolution
			float resolution;

			//uint32 width
			uint32 width;

			//uint32 height
			uint32 height;

			//geometry_msgs/Pose origin
			geometry_msgs::Pose origin;
		};
	}
}
