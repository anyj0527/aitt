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

#pragma once

#include <AITT.h>
#include <MainLoopHandler.h>
#include <TransportModule.h>

#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>

#include "PublishStream.h"
#include "SubscribeStream.h"

using AITT = aitt::AITT;
using TransportModule = aitt::TransportModule;
using MainLoopHandler = aitt::MainLoopHandler;

class Module : public TransportModule {
  public:
    explicit Module(const std::string &ip);
    virtual ~Module(void);

    // TODO: How about regarding topic as service name?
    void Publish(const std::string &topic, const void *data, const size_t datalen,
          const std::string &correlation, AittQoS qos = AITT_QOS_AT_MOST_ONCE,
          bool retain = false) override;

    void Publish(const std::string &topic, const void *data, const size_t datalen,
          AittQoS qos = AITT_QOS_AT_MOST_ONCE, bool retain = false) override;

    // TODO: How about regarding topic as service name?
    void *Subscribe(const std::string &topic, const TransportModule::SubscribeCallback &cb,
          void *cbdata = nullptr, AittQoS qos = AITT_QOS_AT_MOST_ONCE) override;

    void *Subscribe(const std::string &topic, const TransportModule::SubscribeCallback &cb,
          const void *data, const size_t datalen, void *cbdata = nullptr,
          AittQoS qos = AITT_QOS_AT_MOST_ONCE) override;

    void *Unsubscribe(void *handle) override;

    // NOTE:
    // The following callback is going to be called when there is a message of the discovery
    // information The callback will be called by the AITT implementation
    void DiscoveryMessageCallback(const std::string &clientId, const std::string &status,
          const void *msg, const int szmsg) override;

    // NOTE:
    // AITT implementation could call this method to get the discovery message to broadcast it
    // through the MQTT broker
    void GetDiscoveryMessage(const void *&msg, int &szmsg) override;
    // NOTE:
    // If we are able to use a string for the protocol,
    // the module can be developed more freely.
    // even if modules based on the same protocol, implementations can be different.
    AittProtocol GetProtocol(void) override;

  private:
    Config BuildConfigFromFb(const void *data, const size_t data_size);

    std::map<std::string, std::shared_ptr<PublishStream>> publish_table_;
    std::mutex publish_table_lock_;
    std::map<std::string, std::shared_ptr<SubscribeStream>> subscribe_table_;
    std::mutex subscribe_table_lock_;
};
