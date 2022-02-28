#include "BuiltinInterfacesTimeConverter.h"


UBuiltinInterfacesTimeConverter::UBuiltinInterfacesTimeConverter()
{
	_MessageType = "builtin_interfaces/Time";
}

bool UBuiltinInterfacesTimeConverter::ConvertIncomingMessage(const ROSBridgePublishMsg* message, TSharedPtr<FROSBaseMsg> &BaseMsg)
{
	auto msg = new ROSMessages::builtin_interfaces::Time();
	BaseMsg = TSharedPtr<FROSBaseMsg>(msg);
	return _bson_extract_child_time(message->full_msg_bson_, "msg", msg);
}

bool UBuiltinInterfacesTimeConverter::ConvertOutgoingMessage(TSharedPtr<FROSBaseMsg> BaseMsg, bson_t** message)
{
	auto CastMsg = StaticCastSharedPtr<ROSMessages::builtin_interfaces::Time>(BaseMsg);
	*message = bson_new();
	_bson_append_time(*message, CastMsg.Get());
	return true;
}
