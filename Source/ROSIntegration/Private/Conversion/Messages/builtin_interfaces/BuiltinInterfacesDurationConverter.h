#pragma once

#include "CoreMinimal.h"
#include "Conversion/Messages/BaseMessageConverter.h"
#include "builtin_interfaces/Duration.h"

#include "BuiltinInterfacesDurationConverter.generated.h"


UCLASS()
class ROSINTEGRATION_API UBuiltinInterfacesDurationConverter : public UBaseMessageConverter
{
	GENERATED_BODY()

public:
	UBuiltinInterfacesDurationConverter();

	virtual bool ConvertIncomingMessage(const ROSBridgePublishMsg* message, TSharedPtr<FROSBaseMsg> &BaseMsg);
	virtual bool ConvertOutgoingMessage(TSharedPtr<FROSBaseMsg> BaseMsg, bson_t** message);

	static bool _bson_extract_child_duration(bson_t *b, FString key, ROSMessages::builtin_interfaces::Duration *msg, bool LogOnErrors = true)
	{
		// TODO Check if rosbridge sends UINT64 or INT32 (there is no uint32 in bson)
		bool KeyFound = false;
		msg->sec = GetInt32FromBSON(key + ".sec", b, KeyFound, LogOnErrors); if (!KeyFound) return false;
		msg->nanosec = GetInt32FromBSON(key + ".nanosec", b, KeyFound, LogOnErrors); if (!KeyFound) return false;
		return true;
	}

	static void _bson_append_child_duration(bson_t *b, const char *key, const ROSMessages::builtin_interfaces::Duration *msg)
	{
		bson_t child;
		BSON_APPEND_DOCUMENT_BEGIN(b, key, &child);
		_bson_append_duration(&child, msg);
		bson_append_document_end(b, &child);
	}

	static void _bson_append_duration(bson_t *b, const ROSMessages::builtin_interfaces::Duration *msg)
	{
		BSON_APPEND_INT32(b, "sec", msg->sec);
		BSON_APPEND_INT32(b, "nanosec", msg->nanosec);
	}
};
