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
#include "NullTransport.h"

#include "aitt_internal.h"

NullTransport::NullTransport(AittDiscovery& discovery, const std::string& ip)
      : AittTransport(AITT_TYPE_UNKNOWN, discovery)
{
}

void NullTransport::Publish(const std::string& topic, const void* data, const size_t datalen,
      const std::string& correlation, AittQoS qos, bool retain)
{
}

void NullTransport::Publish(const std::string& topic, const void* data, const size_t datalen,
      AittQoS qos, bool retain)
{
}

void* NullTransport::Subscribe(const std::string& topic, const SubscribeCallback& cb, void* cbdata,
      AittQoS qos)
{
    return nullptr;
}

void* NullTransport::Subscribe(const std::string& topic, const SubscribeCallback& cb,
      const void* data, const size_t datalen, void* cbdata, AittQoS qos)
{
    return nullptr;
}

void* NullTransport::Unsubscribe(void* handle)
{
    return nullptr;
}
