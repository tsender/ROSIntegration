#pragma once

#include "CoreMinimal.h"
#include "Conversion/Messages/BaseMessageConverter.h"
#include "Conversion/Messages/builtin_interfaces/BuiltinInterfacesTimeConverter.h"
#include "std_msgs/Header.h"
#include "StdMsgsHeaderConverter.generated.h"


UCLASS()
class ROSINTEGRATION_API UStdMsgsHeaderConverter: public UBaseMessageConverter
{
	GENERATED_BODY()

public:
	UStdMsgsHeaderConverter();

	virtual bool ConvertIncomingMessage(const ROSBridgePublishMsg* message, TSharedPtr<FROSBaseMsg> &BaseMsg);
	virtual bool ConvertOutgoingMessage(TSharedPtr<FROSBaseMsg> BaseMsg, bson_t** message);

	static bool _bson_extract_child_header(bson_t *b, FString key, ROSMessages::std_msgs::Header *msg, bool LogOnErrors = true)
	{
		// TODO Check if rosbridge sends UINT64 or INT32 (there is no uint32 in bson)
		bool KeyFound = false;
		KeyFound = UBuiltinInterfacesTimeConverter::_bson_extract_child_time(b, key + ".stamp", &msg->stamp, LogOnErrors); if (!KeyFound) return false;
		msg->frame_id = GetFStringFromBSON(key + ".frame_id", b, KeyFound, LogOnErrors); if (!KeyFound) return false;
		return true;
	}

	static void _bson_append_child_header(bson_t* b, const char* key, const ROSMessages::std_msgs::Header* msg)
	{
		bson_t child;
		BSON_APPEND_DOCUMENT_BEGIN(b, key, &child);
		_bson_append_header(&child, msg);
		bson_append_document_end(b, &child);
	}

	static void _bson_append_header(bson_t *b, const ROSMessages::std_msgs::Header *msg)
	{
		UBuiltinInterfacesTimeConverter::_bson_append_child_time(b, "stamp", &msg->stamp);
		BSON_APPEND_UTF8(b, "frame_id", TCHAR_TO_UTF8(*msg->frame_id));
	}
};
