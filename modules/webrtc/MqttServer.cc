/*
 * Copyright (c) 2022 Samsung Electronics Co., Ltd All Rights Reserved
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "MqttServer.h"

#include "log.h"

#define MQTT_HANDLER_DEFAULT_QOS 2

MqttServer::MqttServer(const Config &config)
{
    broker_ip_ = config.GetBrokerIp();
    broker_port_ = config.GetBrokerPort();
    id_ = config.GetLocalId();
    room_id_ = config.GetRoomId();
    source_id_ = config.GetSourceId();
    is_publisher_ = (id_ == source_id_);

    do {
        DBG("ID[%s] BROKER IP[%s] BROKER PORT [%d] ROOM[%s] %s", id_.c_str(), broker_ip_.c_str(),
              broker_port_, room_id_.c_str(), is_publisher_ ? "Publisher" : "Subscriber");

        handle_ = mosquitto_new(id_.c_str(), true, this);
        if (handle_ == nullptr) {
            ERR("mosquitto_new() Fail");
            break;
        }

        int ret = mosquitto_int_option(handle_, MOSQ_OPT_PROTOCOL_VERSION, MQTT_PROTOCOL_V5);
        if (ret != MOSQ_ERR_SUCCESS) {
            ERR("mosquitto_int_option() Fail(%s)", mosquitto_strerror(ret));
            break;
        }

        mosquitto_connect_callback_set(handle_, OnConnect);
        mosquitto_disconnect_callback_set(handle_, OnDisconnect);
        mosquitto_message_v5_callback_set(handle_, MessageCallback);

        ret = mosquitto_loop_start(handle_);
        if (ret != MOSQ_ERR_SUCCESS) {
            ERR("mosquitto_loop_start() Fail(%s)", mosquitto_strerror(ret));
            break;
        }

        return;
    } while (0);

    mosquitto_destroy(handle_);
    throw std::runtime_error("MqttHandler Constructor Error");
}

MqttServer::~MqttServer()
{
    int ret = mosquitto_loop_stop(handle_, true);
    if (ret != MOSQ_ERR_SUCCESS)
        ERR("mosquitto_loop_stop() Fail(%s)", mosquitto_strerror(ret));

    mosquitto_destroy(handle_);

    ret = mosquitto_lib_cleanup();
    if (ret != MOSQ_ERR_SUCCESS)
        ERR("Failed to cleanup the mqtt library (%s)", mosquitto_strerror(ret));
}

void MqttServer::SetConnectionState(ConnectionState state)
{
    connection_state_ = state;
    if (connection_state_changed_cb_)
        connection_state_changed_cb_(state);
}

void MqttServer::MessageCallback(mosquitto *handle, void *mqtt_server, const mosquitto_message *msg,
      const mosquitto_property *props)
{
    if (msg->payloadlen == 0) {
        DBG("Zero payload received");
        return;
    }

    INFO("%s received",
          std::string(static_cast<char *>(msg->payload), msg->payloadlen - 1).c_str());
    MqttServer *server = static_cast<MqttServer *>(mqtt_server);
    if (!server)
        return;

    if (server->IsRoomTopic(msg->topic))
        server->HandleRoomTopic(
              std::string(static_cast<char *>(msg->payload), msg->payloadlen - 1).c_str());
    else if (server->IsSourceTopic(msg->topic))
        server->HandleSourceTopic(
              std::string(static_cast<char *>(msg->payload), msg->payloadlen - 1).c_str(),
              msg->retain);
    else if (server->IsMessageTopic(msg->topic))
        server->HandleMessageTopic(
              std::string(static_cast<char *>(msg->payload), msg->payloadlen - 1).c_str());
    else
        ERR("Can't handle this topic yet %s", msg->topic);
}

void MqttServer::HandleRoomTopic(const std::string &message)
{
    INFO("Room topic");
    std::string peer_id;
    if (message.compare(0, 16, "ROOM_PEER_JOINED") == 0) {
        peer_id = message.substr(17, std::string::npos);
    } else if (message.compare(0, 14, "ROOM_PEER_LEFT") == 0) {
        peer_id = message.substr(15, std::string::npos);
    } else {
        ERR("Invalid type of Room message %s", message.c_str());
        return;
    }

    if (peer_id == id_) {
        ERR("ignore");
        return;
    }

    if (is_publisher_) {
        if (room_message_arrived_cb_)
            room_message_arrived_cb_(message);
    } else {
        // TODO: ADHOC, will handle this by room
        if (peer_id != source_id_) {
            ERR("Not source");
            return;
        }

        if (room_message_arrived_cb_)
            room_message_arrived_cb_(message);
    }
}

void MqttServer::HandleSourceTopic(const std::string &message, bool is_retain_message)
{
    INFO("Source topic");
    if (is_publisher_) {
        ERR("Ignore");
    } else {
        ERR("Set source ID %s", message.c_str());
        SetSourceId(message);
        SetConnectionState(ConnectionState::Registered);
    }
}

void MqttServer::HandleMessageTopic(const std::string &message)
{
    INFO("Message topic");
    if (room_message_arrived_cb_)
        room_message_arrived_cb_(message);
}

void MqttServer::OnConnect(mosquitto *handle, void *mqtt_server, int code)
{
    INFO("Connected to signalling server");
    // TODO
    MqttServer *server = static_cast<MqttServer *>(mqtt_server);
    if (!server)
        return;

    // Sometimes it seems that broker is silently disconnected/reconnected
    if (server->GetConnectionState() != ConnectionState::Connecting) {
        ERR("Invalid status");
        return;
    }

    server->SetConnectionState(ConnectionState::Connected);
    server->SetConnectionState(ConnectionState::Registering);
    try {
        server->RegisterWithServer();
    } catch (const std::runtime_error &e) {
        ERR("%s", e.what());
        server->SetConnectionState(ConnectionState::Connected);
    }
}

void MqttServer::RegisterWithServer(void)
{
    if (connection_state_ != IfaceServer::ConnectionState::Registering) {
        ERR("Invaild status");
        throw std::runtime_error("Invalid status");
        return;
    }

    // Notify Who is source?
    std::string source_topic = room_id_ + std::string("/source");
    if (is_publisher_) {
        int ret = mosquitto_publish(handle_, nullptr, source_topic.c_str(), id_.size() + 1,
              id_.c_str(), MQTT_HANDLER_DEFAULT_QOS, true);
        if (ret != MOSQ_ERR_SUCCESS) {
            ERR("mosquitto_publish(%s) Fail(%s)", source_topic.c_str(), mosquitto_strerror(ret));
            throw std::runtime_error(mosquitto_strerror(ret));
        }
        SetConnectionState(ConnectionState::Registered);
    } else {
        int ret =
              mosquitto_subscribe(handle_, nullptr, source_topic.c_str(), MQTT_HANDLER_DEFAULT_QOS);
        if (ret != MOSQ_ERR_SUCCESS) {
            ERR("mosquitto_subscribe() Fail(%s)", mosquitto_strerror(ret));
            throw std::runtime_error(mosquitto_strerror(ret));
        }
    }
}

void MqttServer::OnDisconnect(mosquitto *handle, void *mqtt_server, int code)
{
    INFO("mosquitto disconnected(%s)", mosquitto_strerror(code));
    MqttServer *server = static_cast<MqttServer *>(mqtt_server);
    if (!server)
        return;

    server->SetConnectionState(ConnectionState::Disconnected);
    // TODO
}

bool MqttServer::IsConnected(void)
{
    INFO("%s", __func__);

    return connection_state_ == IfaceServer::ConnectionState::Registered;
}

int MqttServer::Connect(void)
{
    std::string will_message = std::string("ROOM_PEER_LEFT ") + id_;
    int ret = mosquitto_will_set(handle_, room_id_.c_str(), will_message.size() + 1,
          will_message.c_str(), 2, false);
    if (ret != MOSQ_ERR_SUCCESS)
        throw std::runtime_error(mosquitto_strerror(ret));

    SetConnectionState(ConnectionState::Connecting);
    ret = mosquitto_connect(handle_, broker_ip_.c_str(), broker_port_, 60);
    if (ret != MOSQ_ERR_SUCCESS)
        throw std::runtime_error(mosquitto_strerror(ret));

    return 0;
}

int MqttServer::Disconnect(void)
{
    if (is_publisher_) {
        INFO("remove retained");
        // There're some differences between Qos 1 and Qos 2...
        std::string source_topic = room_id_ + std::string("/source");
        int ret = mosquitto_publish(handle_, nullptr, source_topic.c_str(), 0, nullptr, 1, true);
        if (ret != MOSQ_ERR_SUCCESS) {
            ERR("mosquitto_publish(%s) Fail(%s)", source_topic.c_str(), mosquitto_strerror(ret));
            throw std::runtime_error(mosquitto_strerror(ret));
        }
    }
    // Need PEER_LEFT message or use Last will?
    std::string left_message = std::string("ROOM_PEER_LEFT ") + id_;
    int ret = mosquitto_publish(handle_, nullptr, room_id_.c_str(), left_message.size() + 1,
          left_message.c_str(), 1, false);
    if (ret != MOSQ_ERR_SUCCESS)
        throw std::runtime_error(mosquitto_strerror(ret));

    ret = mosquitto_disconnect(handle_);
    if (ret != MOSQ_ERR_SUCCESS)
        throw std::runtime_error(mosquitto_strerror(ret));

    room_id_ = std::string("");

    SetConnectionState(ConnectionState::Disconnected);
    return 0;
}

int MqttServer::SendMessage(const std::string &peer_id, const std::string &msg)
{
    if (!handle_ || room_id_.empty()) {
        ERR("Invaild status");
        return -1;
    }
    if (peer_id.size() == 0 || msg.size() == 0) {
        ERR("Invalid parameter");
        return -1;
    }

    std::string receiver_topic = room_id_ + std::string("/") + peer_id;
    std::string mqtt_server_formatted_message =
          std::string("ROOM_PEER_MSG ") + id_ + std::string(" ") + msg;
    int ret = mosquitto_publish(handle_, nullptr, receiver_topic.c_str(),
          mqtt_server_formatted_message.size() + 1, mqtt_server_formatted_message.c_str(),
          1, false);
    if (ret != MOSQ_ERR_SUCCESS) {
        ERR("mosquitto_publish(%s) Fail(%s)", room_id_.c_str(), mosquitto_strerror(ret));
        return -1;
    }

    return 0;
}

std::string MqttServer::GetConnectionStateStr(ConnectionState state)
{
    std::string state_str;
    switch (state) {
    case IfaceServer::ConnectionState::Disconnected: {
        state_str = std::string("Disconnected");
        break;
    }
    case IfaceServer::ConnectionState::Connecting: {
        state_str = std::string("Connecting");
        break;
    }
    case IfaceServer::ConnectionState::Connected: {
        state_str = std::string("Connected");
        break;
    }
    case IfaceServer::ConnectionState::Registering: {
        state_str = std::string("Registering");
        break;
    }
    case IfaceServer::ConnectionState::Registered: {
        state_str = std::string("Registered");
        break;
    }
    }

    return state_str;
}

void MqttServer::JoinRoom(const std::string &room_id)
{
    if (room_id.empty() || room_id != room_id_) {
        ERR("Invaild room id");
        throw std::runtime_error(std::string("Invalid room_id"));
        return;
    }

    // Subscribe PEER_JOIN PEER_LEFT
    int ret = mosquitto_subscribe(handle_, nullptr, room_id_.c_str(), MQTT_HANDLER_DEFAULT_QOS);
    if (ret != MOSQ_ERR_SUCCESS) {
        ERR("mosquitto_subscribe() Fail(%s)", mosquitto_strerror(ret));
        throw std::runtime_error(mosquitto_strerror(ret));
    }

    // Subscribe PEER_MSG
    std::string receiving_topic = room_id + std::string("/") + id_;
    ret = mosquitto_subscribe(handle_, nullptr, receiving_topic.c_str(), 1);
    if (ret != MOSQ_ERR_SUCCESS) {
        ERR("mosquitto_subscribe() Fail(%s)", mosquitto_strerror(ret));
        throw std::runtime_error(mosquitto_strerror(ret));
    }
    INFO("Subscribe room topics");

    if (!is_publisher_) {
        std::string join_message = std::string("ROOM_PEER_JOINED ") + id_;
        ret = mosquitto_publish(handle_, nullptr, room_id_.c_str(), join_message.size() + 1,
            join_message.c_str(), MQTT_HANDLER_DEFAULT_QOS, false);
        if (ret != MOSQ_ERR_SUCCESS)
            throw std::runtime_error(mosquitto_strerror(ret));
    }
}
