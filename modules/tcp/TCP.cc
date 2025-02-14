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
#include "TCP.h"

#include <AittTypes.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cstdlib>
#include <cstring>
#include <stdexcept>

#include "aitt_internal.h"

namespace AittTCPNamespace {

TCP::TCP(const std::string &host, const ConnectInfo &connect_info)
      : handle(-1), addrlen(0), addr(nullptr), secure(false)
{
    int ret = 0;

    do {
        if (connect_info.port == 0) {
            ret = EINVAL;
            break;
        }

        handle = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
        if (handle < 0) {
            ERR("socket() Fail()");
            break;
        }

        addrlen = sizeof(sockaddr_in);
        addr = static_cast<sockaddr *>(calloc(1, addrlen));
        if (!addr) {
            ERR("calloc() Fail()");
            break;
        }

        sockaddr_in *inet_addr = reinterpret_cast<sockaddr_in *>(addr);
        if (!inet_pton(AF_INET, host.c_str(), &inet_addr->sin_addr)) {
            ret = EINVAL;
            break;
        }

        inet_addr->sin_port = htons(connect_info.port);
        inet_addr->sin_family = AF_INET;

        ret = connect(handle, addr, addrlen);
        if (ret < 0) {
            ERR("connect() Fail(%s, %d)", host.c_str(), connect_info.port);
            break;
        }

        SetupOptions(connect_info);
        return;
    } while (0);

    if (ret <= 0)
        ret = errno;

    free(addr);
    if (handle >= 0 && close(handle) < 0)
        ERR_CODE(errno, "close");
    throw std::runtime_error(strerror(ret));
}

TCP::TCP(int handle, sockaddr *addr, socklen_t szAddr, const ConnectInfo &connect_info)
      : handle(handle), addrlen(szAddr), addr(addr), secure(false)
{
    SetupOptions(connect_info);
}

TCP::~TCP(void)
{
    if (handle < 0)
        return;

    free(addr);
    if (close(handle) < 0)
        ERR_CODE(errno, "close");
}

void TCP::SetupOptions(const ConnectInfo &connect_info)
{
    int on = 1;

    int ret = setsockopt(handle, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
    if (ret < 0) {
        ERR_CODE(errno, "delay option setting failed");
    }

    if (connect_info.secure) {
        secure = true;
        crypto.Init(connect_info.key, connect_info.iv);
    }
}

void TCP::Send(const void *data, size_t &szData)
{
    size_t sent = 0;
    while (sent < szData) {
        int ret = send(handle, static_cast<const char *>(data) + sent, szData - sent, 0);
        if (ret < 0) {
            ERR("Fail to send data, handle = %d, size = %zu", handle, szData);
            throw std::runtime_error(strerror(errno));
        }

        sent += ret;
    }
    szData = sent;
}

void TCP::SendSizedData(const void *data, size_t &szData)
{
    if (secure)
        SendSizedDataSecure(data, szData);
    else
        SendSizedDataNormal(data, szData);
}

int TCP::Recv(void *data, size_t &szData)
{
    size_t received = 0;
    while (received < szData) {
        int ret = recv(handle, static_cast<char *>(data) + received, szData - received, 0);
        if (ret < 0) {
            ERR("Fail to recv data, handle = %d, size = %zu", handle, szData);
            throw std::runtime_error(strerror(errno));
        }
        if (ret == 0) {
            ERR("disconnected");
            return -1;
        }

        received += ret;
    }

    szData = received;
    return 0;
}

int TCP::RecvSizedData(void **data, size_t &szData)
{
    if (secure)
        return RecvSizedDataSecure(data, szData);
    else
        return RecvSizedDataNormal(data, szData);
}

int TCP::HandleZeroMsg(void **data, size_t &data_size)
{
    // distinguish between connection problems and zero-size messages
    INFO("Got a zero-size message.");
    data_size = 0;
    *data = nullptr;
    return 0;
}

int TCP::GetHandle(void)
{
    return handle;
}

void TCP::GetPeerInfo(std::string &host, unsigned short &port)
{
    char address[INET_ADDRSTRLEN] = {
          0,
    };

    if (!inet_ntop(AF_INET, &reinterpret_cast<sockaddr_in *>(this->addr)->sin_addr, address,
              sizeof(address)))
        throw std::runtime_error(strerror(errno));

    port = ntohs(reinterpret_cast<sockaddr_in *>(this->addr)->sin_port);
    host = address;
}

unsigned short TCP::GetPort(void)
{
    sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);

    if (getsockname(handle, reinterpret_cast<sockaddr *>(&addr), &addrlen) < 0)
        throw std::runtime_error(strerror(errno));

    return ntohs(addr.sin_port);
}

void TCP::SendSizedDataNormal(const void *data, size_t &data_size)
{
    size_t fixed_data_size = data_size;
    if (0 == data_size) {
        // distinguish between connection problems and zero-size messages
        INFO("Send a zero-size message.");
        fixed_data_size = UINT32_MAX;
    }

    size_t size_len = sizeof(fixed_data_size);
    Send(static_cast<const void *>(&fixed_data_size), size_len);
    Send(data, data_size);
}

int TCP::RecvSizedDataNormal(void **data, size_t &data_size)
{
    int ret;

    size_t data_len = 0;
    size_t size_len = sizeof(data_len);
    ret = Recv(static_cast<void *>(&data_len), size_len);
    if (ret < 0) {
        ERR("Recv() Fail(%d)", ret);
        return ret;
    }

    if (data_len == UINT32_MAX)
        return HandleZeroMsg(data, data_size);

    void *data_buf = malloc(data_len);
    Recv(data_buf, data_len);
    data_size = data_len;
    *data = data_buf;

    return 0;
}

void TCP::SendSizedDataSecure(const void *data, size_t &data_size)
{
    size_t fixed_data_size = data_size;
    if (0 == data_size) {
        // distinguish between connection problems and zero-size messages
        INFO("Send a zero-size message.");
        fixed_data_size = UINT32_MAX;
    }

    size_t size_len;
    if (data_size) {
        unsigned char data_buf[crypto.GetCryptogramSize(data_size)];
        size_t data_len =
              crypto.Encrypt(static_cast<const unsigned char *>(data), data_size, data_buf);
        unsigned char size_buf[crypto.GetCryptogramSize(sizeof(size_t))];
        size_len = crypto.Encrypt((unsigned char *)&data_len, sizeof(data_len), size_buf);
        Send(size_buf, size_len);
        Send(data_buf, data_len);
    } else {
        unsigned char size_buf[crypto.GetCryptogramSize(sizeof(size_t))];
        size_len =
              crypto.Encrypt((unsigned char *)&fixed_data_size, sizeof(fixed_data_size), size_buf);
        Send(size_buf, size_len);
    }
}

int TCP::RecvSizedDataSecure(void **data, size_t &data_size)
{
    int ret;

    size_t cipher_size_len = crypto.GetCryptogramSize(sizeof(size_t));
    unsigned char cipher_size_buf[cipher_size_len];
    ret = Recv(cipher_size_buf, cipher_size_len);
    if (ret < 0) {
        ERR("Recv() Fail(%d)", ret);
        return ret;
    }

    unsigned char plain_size_buf[cipher_size_len];
    size_t cipher_data_len = 0;
    crypto.Decrypt(cipher_size_buf, cipher_size_len, plain_size_buf);
    memcpy(&cipher_data_len, plain_size_buf, sizeof(cipher_data_len));
    if (cipher_data_len == UINT32_MAX)
        return HandleZeroMsg(data, data_size);

    if (AITT_MESSAGE_MAX < cipher_data_len) {
        ERR("Invalid Size(%zu)", cipher_data_len);
        return -1;
    }
    unsigned char cipher_data_buf[cipher_data_len];
    Recv(cipher_data_buf, cipher_data_len);
    unsigned char *data_buf = static_cast<unsigned char *>(malloc(cipher_data_len));
    data_size = crypto.Decrypt(cipher_data_buf, cipher_data_len, data_buf);
    *data = data_buf;
    return 0;
}

TCP::ConnectInfo::ConnectInfo() : port(0), secure(false), key(), iv()
{
}

}  // namespace AittTCPNamespace
