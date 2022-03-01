#pragma once

#include "CoreMinimal.h"
#include "Conversion/Messages/BaseMessageConverter.h"
#include "Conversion/Messages/builtin_interfaces/BuiltinInterfacesTimeConverter.h"
#include "Conversion/Messages/geometry_msgs/GeometryMsgsPoseConverter.h"
#include "nav_msgs/MapMetaData.h"
#include "NavMsgsMapMetaDataConverter.generated.h"


UCLASS()
class ROSINTEGRATION_API UNavMsgsMapMetaDataConverter : public UBaseMessageConverter
{
	GENERATED_BODY()
	
public:
	UNavMsgsMapMetaDataConverter();
	virtual bool ConvertIncomingMessage(const ROSBridgePublishMsg* message, TSharedPtr<FROSBaseMsg> &BaseMsg);
	virtual bool ConvertOutgoingMessage(TSharedPtr<FROSBaseMsg> BaseMsg, bson_t** message);
	
	static bool _bson_extract_child_map_meta_data(bson_t *b, FString key, ROSMessages::nav_msgs::MapMetaData *msg)
	{
		bool KeyFound = false;

		if (!UBuiltinInterfacesTimeConverter::_bson_extract_child_time(b, key + ".map_load_time", &msg->map_load_time)) return false;
		msg->resolution = (float)GetDoubleFromBSON(key + ".resolution", b, KeyFound);  if (!KeyFound) return false;
		msg->width = GetInt32FromBSON(key + ".width", b, KeyFound); if (!KeyFound) return false;
		msg->height = GetInt32FromBSON(key + ".height", b, KeyFound); if (!KeyFound) return false;
		if (!UGeometryMsgsPoseConverter::_bson_extract_child_pose(b, key + ".origin", &msg->origin)) return false;
		
		return true;
	}
	
	static void _bson_append_child_map_meta_data(bson_t *b, const char *key, const ROSMessages::nav_msgs::MapMetaData *msg)
	{
		bson_t mapmetadata;
		BSON_APPEND_DOCUMENT_BEGIN(b, key, &mapmetadata);
		_bson_append_map_meta_data(&mapmetadata, msg);
		bson_append_document_end(b, &mapmetadata);
	}
	
	static void _bson_append_map_meta_data(bson_t *b, const ROSMessages::nav_msgs::MapMetaData *msg)
	{
		UBuiltinInterfacesTimeConverter::_bson_append_child_time(b, "map_load_time", &msg->map_load_time);	
		BSON_APPEND_DOUBLE(b, "resolution", msg->resolution);
		BSON_APPEND_INT32(b, "width",  msg->width);
		BSON_APPEND_INT32(b, "height", msg->height);
		UGeometryMsgsPoseConverter::_bson_append_child_pose(b, "origin", &msg->origin);
	}
	
};

