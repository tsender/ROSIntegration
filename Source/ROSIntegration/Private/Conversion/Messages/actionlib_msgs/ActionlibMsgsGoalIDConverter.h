#pragma once

#include "CoreMinimal.h"
#include "Conversion/Messages/BaseMessageConverter.h"
#include "Conversion/Messages/builtin_interfaces/BuiltinInterfacesTimeConverter.h"
#include "actionlib_msgs/GoalID.h"
#include "ActionlibMsgsGoalIDConverter.generated.h"


UCLASS()
class ROSINTEGRATION_API UActionlibMsgsGoalIDConverter : public UBaseMessageConverter
{
	GENERATED_BODY()

public:
	UActionlibMsgsGoalIDConverter();
	virtual bool ConvertIncomingMessage(const ROSBridgePublishMsg* message, TSharedPtr<FROSBaseMsg> &BaseMsg);
	virtual bool ConvertOutgoingMessage(TSharedPtr<FROSBaseMsg> BaseMsg, bson_t** message);

	static bool _bson_extract_child_goal_id(bson_t *b, FString key, ROSMessages::actionlib_msgs::GoalID *msg, bool LogOnErrors = true)
	{
		bool KeyFound = false;

		if (!UBuiltinInterfacesTimeConverter::_bson_extract_child_time(b, key + ".stamp", &msg->stamp)) return false;
		msg->id = GetFStringFromBSON(key + ".id", b, KeyFound, LogOnErrors); if (!KeyFound) return false;

		return true;
	}

	static void _bson_append_child_goal_id(bson_t *b, const char *key, const ROSMessages::actionlib_msgs::GoalID *msg)
	{
		bson_t child;
		BSON_APPEND_DOCUMENT_BEGIN(b, key, &child);
		_bson_append_goal_id(&child, msg);
		bson_append_document_end(b, &child);
	}

	static void _bson_append_goal_id(bson_t *b, const ROSMessages::actionlib_msgs::GoalID *msg)
	{
		UBuiltinInterfacesTimeConverter::_bson_append_child_time(b, "stamp", &msg->stamp);
		BSON_APPEND_UTF8(b, "id", TCHAR_TO_UTF8(*msg->id));
	}
};
