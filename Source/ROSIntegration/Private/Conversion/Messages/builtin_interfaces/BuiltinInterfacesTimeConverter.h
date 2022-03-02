#pragma once

#include "CoreMinimal.h"
#include "Conversion/Messages/BaseMessageConverter.h"
#include "builtin_interfaces/Time.h"
#include "BuiltinInterfacesTimeConverter.generated.h"


UCLASS()
class ROSINTEGRATION_API UBuiltinInterfacesTimeConverter : public UBaseMessageConverter
{
	GENERATED_BODY()

public:
	UBuiltinInterfacesTimeConverter();

	virtual bool ConvertIncomingMessage(const ROSBridgePublishMsg* message, TSharedPtr<FROSBaseMsg> &BaseMsg);
	virtual bool ConvertOutgoingMessage(TSharedPtr<FROSBaseMsg> BaseMsg, bson_t** message);

	static bool _bson_extract_child_time(bson_t *b, FString key, ROSMessages::builtin_interfaces::Time *msg, bool LogOnErrors = true)
	{
		// TODO Check if rosbridge sends UINT64 or INT32 (there is no uint32 in bson)
		bool KeyFound = false;
		msg->sec = GetInt32FromBSON(key + ".sec", b, KeyFound, LogOnErrors); if (!KeyFound) return false;
		msg->nanosec = GetInt32FromBSON(key + ".nanosec", b, KeyFound, LogOnErrors); if (!KeyFound) return false;
		return true;
	}

	static void _bson_append_child_time(bson_t *b, const char *key, const ROSMessages::builtin_interfaces::Time *msg)
	{
		bson_t child;
		BSON_APPEND_DOCUMENT_BEGIN(b, key, &child);
		_bson_append_time(&child, msg);
		bson_append_document_end(b, &child);
	}

	static void _bson_append_time(bson_t *b, const ROSMessages::builtin_interfaces::Time *msg)
	{
		BSON_APPEND_INT32(b, "sec", msg->sec);
		BSON_APPEND_INT32(b, "nanosec", msg->nanosec);
	}
};
