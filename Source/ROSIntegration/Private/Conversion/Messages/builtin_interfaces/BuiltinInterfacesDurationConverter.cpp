#include "BuiltinInterfacesDurationConverter.h"


UBuiltinInterfacesDurationConverter::UBuiltinInterfacesDurationConverter()
{
	_MessageType = "builtin_interfaces/Duration";
}

bool UBuiltinInterfacesDurationConverter::ConvertIncomingMessage(const ROSBridgePublishMsg* message, TSharedPtr<FROSBaseMsg> &BaseMsg)
{
	auto msg = new ROSMessages::builtin_interfaces::Duration();
	BaseMsg = TSharedPtr<FROSBaseMsg>(msg);
	return _bson_extract_child_duration(message->full_msg_bson_, "msg", msg);
}

bool UBuiltinInterfacesDurationConverter::ConvertOutgoingMessage(TSharedPtr<FROSBaseMsg> BaseMsg, bson_t** message)
{
	auto CastMsg = StaticCastSharedPtr<ROSMessages::builtin_interfaces::Duration>(BaseMsg);
	*message = bson_new();
	_bson_append_duration(*message, CastMsg.Get());
	return true;
}
