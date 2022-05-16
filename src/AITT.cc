/*
 * Copyright (c) 2021-2022 Samsung Electronics Co., Ltd All Rights Reserved
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
#include <sys/random.h>

#include <memory>

#include "AITTImpl.h"
#include "log.h"

namespace aitt {

AITT::AITT(const std::string &id, const std::string &ip_addr, bool clear_session)
{
    std::string valid_id = id;
    std::string valid_ip = ip_addr;

    if (id.empty()) {
        const char character_set[] =
              "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
        char random_idx[16];
        int rc = getrandom(random_idx, sizeof(random_idx), 0);
        if (rc != sizeof(random_idx)) {
            INFO("getrandom() = %d", rc);
        }

        char name[16];
        for (size_t i = 0; i < sizeof(name); i++) {
            name[i] = character_set[random_idx[i] % (sizeof(character_set))];
        }
        valid_id = "aitt-" + std::string(name, sizeof(name) - 1);
        DBG("Generated name = %s", valid_id.c_str());
    }

    if (ip_addr.empty())
        valid_ip = "127.0.0.1";

    pImpl = std::make_unique<AITT::Impl>(*this, valid_id, valid_ip, clear_session);
}

AITT::~AITT(void)
{
}

void AITT::SetWillInfo(const std::string &topic, const void *data, const size_t datalen,
      AITT::QoS qos, bool retain)
{
    return pImpl->SetWillInfo(topic, data, datalen, qos, retain);
}

void AITT::SetConnectionCallback(ConnectionCallback cb, void *user_data)
{
    return pImpl->SetConnectionCallback(cb, user_data);
}

void AITT::Connect(const std::string &host, int port, const std::string &username,
      const std::string &password)
{
    return pImpl->Connect(host, port, username, password);
}

void AITT::Disconnect(void)
{
    return pImpl->Disconnect();
}

void AITT::Publish(const std::string &topic, const void *data, const size_t datalen,
      AittProtocol protocols, AITT::QoS qos, bool retain)
{
    return pImpl->Publish(topic, data, datalen, protocols, qos, retain);
}

int AITT::PublishWithReply(const std::string &topic, const void *data, const size_t datalen,
      AittProtocol protocol, AITT::QoS qos, bool retain, const SubscribeCallback &cb, void *cbdata,
      const std::string &correlation)
{
    return pImpl->PublishWithReply(topic, data, datalen, protocol, qos, retain, cb, cbdata,
          correlation);
}

int AITT::PublishWithReplySync(const std::string &topic, const void *data, const size_t datalen,
      AittProtocol protocol, AITT::QoS qos, bool retain, const SubscribeCallback &cb, void *cbdata,
      const std::string &correlation, int timeout_ms)
{
    return pImpl->PublishWithReplySync(topic, data, datalen, protocol, qos, retain, cb, cbdata,
          correlation, timeout_ms);
}

AittSubscribeID AITT::Subscribe(const std::string &topic, const SubscribeCallback &cb, void *cbdata,
      AittProtocol protocols, AITT::QoS qos)
{
    return pImpl->Subscribe(topic, cb, cbdata, protocols, qos);
}

void *AITT::Unsubscribe(AittSubscribeID handle)
{
    return pImpl->Unsubscribe(handle);
}

void AITT::SendReply(MSG *msg, const void *data, const int datalen, bool end)
{
    return pImpl->SendReply(msg, data, datalen, end);
}

bool AITT::CompareTopic(const std::string &left, const std::string &right)
{
    return MQ::CompareTopic(left, right);
}

}  // namespace aitt
