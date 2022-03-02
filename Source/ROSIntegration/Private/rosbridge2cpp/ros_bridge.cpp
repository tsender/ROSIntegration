
#include "ros_bridge.h"
#include "ros_topic.h"
#include <bson.h>
#include "rapidjson/document.h"
using json = rapidjson::Document;

#include "ROSIntegrationCore.h"
#include "WebSocketsModule.h"
#include "IWebSocket.h"

namespace rosbridge2cpp {

	static const std::chrono::seconds SendThreadFreezeTimeout = std::chrono::seconds(5);
	unsigned long ROSCallbackHandle_id_counter = 1;

	ROSBridge::~ROSBridge()
	{
		CloseWebSocket();
		binary_recv_buffer.Empty();
		connected_to_ws_server = false;
		trying_to_connect_to_ws_server = false;
		UE_LOG(LogROS, Display, TEXT("ROSBridge::~ROSBridge()"));
	}

	bool ROSBridge::Init(FString ip_addr, int port, FString path)
	{
		// Create websocket
		ws_server_url = FString::Printf(TEXT("ws://%s:%i%s"), *ip_addr, port, *path);
		web_socket = FWebSocketsModule::Get().CreateWebSocket(ws_server_url, TEXT("ws"));

		// Bind event delegates (gets called in the game thread)
		web_socket->OnConnected().AddRaw(this, &ROSBridge::OnWebSocketConnected);
		web_socket->OnConnectionError().AddRaw(this, &ROSBridge::OnWebSocketConnectionError);
		web_socket->OnClosed().AddRaw(this, &ROSBridge::OnWebSocketClosed);
		web_socket->OnRawMessage().AddRaw(this, &ROSBridge::OnWebSocketRawMessage);

		web_socket->Connect();
		trying_to_connect_to_ws_server = true;
		return true;
	}

	bool ROSBridge::IsHealthy() const
	{
		return connected_to_ws_server;
	}

	bool ROSBridge::IsTryingToConnect() const
	{
		return trying_to_connect_to_ws_server;
	}

	void ROSBridge::CloseWebSocket()
	{
		if (connected_to_ws_server) web_socket->Close();
	}

	void ROSBridge::OnWebSocketConnected()
	{
		connected_to_ws_server = true;
		trying_to_connect_to_ws_server = false;
		UE_LOG(LogROS, Display, TEXT("ROSBridge: Connected to websocket server at %s"), *ws_server_url);
	}

	void ROSBridge::OnWebSocketConnectionError(const FString& Error)
	{
		connected_to_ws_server = false;
		UE_LOG(LogROS, Error, TEXT("ROSBridge: Unable to connect to websocket server at %s. Returned error message: '%s'"), *ws_server_url, *Error);
	}

	void ROSBridge::OnWebSocketClosed(int32 StatusCode, const FString& Reason, bool bWasClean)
	{
		connected_to_ws_server = false;
		if (bWasClean)
		{
			UE_LOG(LogROS, Display, TEXT("ROSBridge: Websocket closed cleanly with status code %i and reason: '%s'."), StatusCode, *Reason);
		}
		else
		{
			UE_LOG(LogROS, Display, TEXT("ROSBridge: Websocket closed uncleanly with status code %i and reason: '%s'."), StatusCode, *Reason);
		}
	}

	void ROSBridge::OnWebSocketMessage(const FString& MessageString)
	{
		json j;
		j.Parse(TCHAR_TO_UTF8(*MessageString));
		IncomingMessageCallback(j);
	}

	void ROSBridge::OnWebSocketRawMessage(const void* Data, SIZE_T Size, SIZE_T BytesRemaining)
	{	
		binary_recv_buffer.Append(static_cast<const uint8*>(Data), Size);

		if (BytesRemaining == 0) // Full message has been received
		{
			bson_t b;
			if (!bson_init_static(&b, binary_recv_buffer.GetData(), binary_recv_buffer.Num())) 
			{
				UE_LOG(LogROS, Error, TEXT("ROSBridge: Error on BSON parse - Ignoring incoming message"));
			}
			else
			{
				IncomingMessageCallback(b);
			}
			binary_recv_buffer.Empty(); // Empty buffer when done so we can start fresh for next incoming message
		}
	}

	bool ROSBridge::SendMessage(std::string &data) 
	{
		if(bson_only_mode())
		{
			bson_t bson;
			bson_error_t error;
			if (!bson_init_from_json(&bson, data.c_str(), -1, &error)) 
			{
				UE_LOG(LogROS, Error, TEXT("ROSBridge: bson_init_from_json() failed: %s"), error.message);
				bson_destroy(&bson);
				return false;
			}

			const uint8_t *bson_data = bson_get_data(&bson);
			uint32_t bson_size = bson.len;
			web_socket->Send(bson_data, bson_size, true); // Queue message on websocket SendQueue
			bson_destroy(&bson);
			return true;
		}
		else
		{
			web_socket->Send(FString(data.c_str()));
			return true;
		}
	}

	bool ROSBridge::SendMessage(json &data)
	{
		std::string str_repr = Helper::get_string_from_rapidjson(data);
		return SendMessage(str_repr);
	}

	bool ROSBridge::SendMessage(ROSBridgeMsg &msg)
	{
		if (bson_only_mode()) 
		{
			bson_t* message = bson_new();
			msg.ToBSON(*message);

			const uint8_t *bson_data = bson_get_data(message);
			uint32_t bson_size = message->len;
			web_socket->Send(bson_data, bson_size, true); // Queue message on websocket SendQueue
			bson_destroy(message);
			return true;
		}
		else
		{
			// Convert ROSBridgeMsg to JSON
			json alloc;
			json message = msg.ToJSON(alloc.GetAllocator());

			std::string str_repr = Helper::get_string_from_rapidjson(message);
			return SendMessage(str_repr);
		}
	}

	void ROSBridge::HandleIncomingPublishMessage(ROSBridgePublishMsg &data)
	{
		spinlock::scoped_lock_wait_for_short_task lock(change_topics_mutex_);

		// Incoming topic message - dispatch to correct callback
		std::string &incoming_topic_name = data.topic_;
		if (registered_topic_callbacks_.find(incoming_topic_name) == registered_topic_callbacks_.end()) {
			std::cerr << "[ROSBridge] Received message for topic " << incoming_topic_name << " where no callback has been registered before" << std::endl;
			return;
		}

		if (bson_only_mode()) {
			if (!data.full_msg_bson_) {
				std::cerr << "[ROSBridge] Received message for topic " << incoming_topic_name << ", but full message field is missing. Aborting" << std::endl;
				return;
			}
		}
		else {
			if (data.msg_json_.IsNull()) {
				std::cerr << "[ROSBridge] Received message for topic " << incoming_topic_name << ", but 'msg' field is missing. Aborting" << std::endl;
				return;
			}
		}

		// Iterate over all registered callbacks for the given topic
		for (auto& topic_callback : registered_topic_callbacks_.find(incoming_topic_name)->second) {
			topic_callback.GetFunction()(data);
		}
		return;
	}

	void ROSBridge::HandleIncomingServiceResponseMessage(ROSBridgeServiceResponseMsg &data)
	{
		std::string &incoming_service_id = data.id_;

		auto service_response_callback_it = registered_service_callbacks_.find(incoming_service_id);

		if (service_response_callback_it == registered_service_callbacks_.end()) {
			std::cerr << "[ROSBridge] Received response for service id " << incoming_service_id << "where no callback has been registered before" << std::endl;
			return;
		}

		// Execute the callback for the given service id
		service_response_callback_it->second(data);

		// Delete the callback.
		// Every call_service will create a new id
		registered_service_callbacks_.erase(service_response_callback_it);

	}

	void ROSBridge::HandleIncomingServiceRequestMessage(ROSBridgeCallServiceMsg &data)
	{
		std::string &incoming_service = data.service_;

		if (bson_only_mode()) {
			auto service_request_callback_it = registered_service_request_callbacks_bson_.find(incoming_service);

			if (service_request_callback_it == registered_service_request_callbacks_bson_.end()) {
				std::cerr << "[ROSBridge] Received service request for service :" << incoming_service << " where no callback has been registered before" << std::endl;
				return;
			}
			service_request_callback_it->second(data);
		}
		else
		{
			auto service_request_callback_it = registered_service_request_callbacks_.find(incoming_service);

			if (service_request_callback_it == registered_service_request_callbacks_.end()) {
				std::cerr << "[ROSBridge] Received service request for service :" << incoming_service << " where no bson callback has been registered before" << std::endl;
				return;
			}
			rapidjson::Document response_allocator;

			// Execute the callback for the given service id
			service_request_callback_it->second(data, response_allocator.GetAllocator());
		}
	}

	void ROSBridge::IncomingMessageCallback(bson_t &bson)
	{
		// Check the message type and dispatch the message properly
		//
		// Incoming Topic messages
		bool key_found = false;

		if (Helper::get_utf8_by_key("op", bson, key_found) == "publish") {
			ROSBridgePublishMsg m;
			if (m.FromBSON(bson)) {
				HandleIncomingPublishMessage(m);
				return;
			}

			std::cerr << "Failed to parse publish message into class. Skipping message." << std::endl;
		}

		// Service responses for service we called earlier
		if (Helper::get_utf8_by_key("op", bson, key_found) == "service_response") {
			ROSBridgeServiceResponseMsg m;
			if (m.FromBSON(bson)) {
				HandleIncomingServiceResponseMessage(m);
				return;
			}
			std::cerr << "Failed to parse service_response message into class. Skipping message." << std::endl;
		}

		// Service Requests to a service that we advertised in ROSService
		if (Helper::get_utf8_by_key("op", bson, key_found) == "call_service") {
			ROSBridgeCallServiceMsg m;
			m.FromBSON(bson);
			HandleIncomingServiceRequestMessage(m);
		}
	}

	void ROSBridge::IncomingMessageCallback(json &data)
	{
		std::string str_repr = Helper::get_string_from_rapidjson(data);

		// Check the message type and dispatch the message properly
		//
		// Incoming Topic messages
		if (std::string(data["op"].GetString(), data["op"].GetStringLength()) == "publish") {
			ROSBridgePublishMsg m;
			if (m.FromJSON(data)) {
				HandleIncomingPublishMessage(m);
				return;
			}

			std::cerr << "Failed to parse publish message into class. Skipping message." << std::endl;
		}

		// Service responses for service we called earlier
		if (std::string(data["op"].GetString(), data["op"].GetStringLength()) == "service_response") {
			ROSBridgeServiceResponseMsg m;
			// m.FromJSON(data);
			if (m.FromJSON(data)) {
				HandleIncomingServiceResponseMessage(m);
				return;
			}
			std::cerr << "Failed to parse service_response message into class. Skipping message." << std::endl;
		}

		// Service Requests to a service that we advertised in ROSService
		if (std::string(data["op"].GetString(), data["op"].GetStringLength()) == "call_service") {
			ROSBridgeCallServiceMsg m;
			m.FromJSON(data);
			HandleIncomingServiceRequestMessage(m);
		}
	}

	void ROSBridge::RegisterTopicCallback(std::string topic_name, ROSCallbackHandle<FunVrROSPublishMsg>& callback_handle)
	{
		spinlock::scoped_lock_wait_for_short_task lock(change_topics_mutex_);
		registered_topic_callbacks_[topic_name].push_back(callback_handle);
	}

	void ROSBridge::RegisterServiceCallback(std::string service_call_id, FunVrROSServiceResponseMsg fun)
	{
		registered_service_callbacks_[service_call_id] = fun;
	}

	void ROSBridge::RegisterServiceRequestCallback(std::string service_name, FunVrROSCallServiceMsgrROSServiceResponseMsgrAllocator fun)
	{
		registered_service_request_callbacks_[service_name] = fun;
	}

	void ROSBridge::RegisterServiceRequestCallback(std::string service_name, FunVrROSCallServiceMsgrROSServiceResponseMsg fun)
	{
		registered_service_request_callbacks_bson_[service_name] = fun;
	}

	bool ROSBridge::UnregisterTopicCallback(std::string topic_name, const ROSCallbackHandle<FunVrROSPublishMsg>& callback_handle)
	{
		spinlock::scoped_lock_wait_for_short_task lock(change_topics_mutex_);

		if (registered_topic_callbacks_.find(topic_name) == registered_topic_callbacks_.end()) {
			std::cerr << "[ROSBridge] UnregisterTopicCallback called but given topic name '" << topic_name << "' not in map." << std::endl;
			return false;
		}

		std::list<ROSCallbackHandle<FunVrROSPublishMsg>> &r_list_of_callbacks = registered_topic_callbacks_.find(topic_name)->second;

		for (std::list<ROSCallbackHandle<FunVrROSPublishMsg>>::iterator topic_callback_it = r_list_of_callbacks.begin();
			topic_callback_it != r_list_of_callbacks.end();
			++topic_callback_it) {

			if (*topic_callback_it == callback_handle) {
				std::cout << "[ROSBridge] Found CB in UnregisterTopicCallback. Deleting it ... " << std::endl;
				r_list_of_callbacks.erase(topic_callback_it);
				return true;
			}
		}
		return false;
	}
}
