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
#include <glib.h>
#include <gtest/gtest.h>
#include <sys/random.h>
#include <sys/time.h>
#include <unistd.h>

#include <thread>

#include "AITT.h"
#include "aitt_internal.h"

#define TEST_MSG "This is my test message"
#define TEST_MSG2 "This message is going to be delivered through a specified AittProtocol"
#define MY_IP "127.0.0.1"
#define SLEEP_MS 1

using AITT = aitt::AITT;

class AITTTCPTest : public testing::Test {
  public:
    void ToggleReady() { ready = true; }
    void ToggleReady2() { ready2 = true; }

    void *subscribeHandle;
    bool ready;
    bool ready2;

  protected:
    void SetUp() override
    {
        ready = false;
        ready2 = false;
        mainLoop = g_main_loop_new(nullptr, FALSE);
        timeval tv;
        char buffer[256];
        gettimeofday(&tv, nullptr);
        snprintf(buffer, sizeof(buffer), "UniqueID.%lX%lX", tv.tv_sec, tv.tv_usec);
        clientId = buffer;
        snprintf(buffer, sizeof(buffer), "TestTopic.%lX%lX", tv.tv_sec, tv.tv_usec);
        testTopic = buffer;
    }

    void IterateEventLoop(void)
    {
        g_main_loop_run(mainLoop);
        DBG("Go forward");
    }

    void TearDown() override { g_main_loop_unref(mainLoop); }

    static gboolean ReadyCheck(gpointer data)
    {
        AITTTCPTest *test = static_cast<AITTTCPTest *>(data);

        if (test->ready) {
            g_main_loop_quit(test->mainLoop);
            return FALSE;
        }

        return TRUE;
    }

    GMainLoop *mainLoop;
    std::string clientId;
    std::string testTopic;
};

TEST_F(AITTTCPTest, TCP_Wildcards1_Anytime)
{
    try {
        char dump_msg[204800];

        AITT aitt(clientId, MY_IP);
        aitt.Connect();

        aitt.Subscribe(
              "test/#",
              [&](aitt::MSG *handle, const void *msg, const size_t szmsg, void *cbdata) -> void {
                  AITTTCPTest *test = static_cast<AITTTCPTest *>(cbdata);
                  INFO("Got Message(Topic:%s, size:%zu)", handle->GetTopic().c_str(), szmsg);
                  static int cnt = 0;
                  ++cnt;
                  if (cnt == 3)
                      test->ToggleReady();
              },
              static_cast<void *>(this), AITT_TYPE_TCP);

        // Wait a few seconds until the AITT client gets a server list (discover devices)
        DBG("Sleep %d secs", SLEEP_MS);
        sleep(SLEEP_MS);

        aitt.Publish("test/step1/value1", dump_msg, 12, AITT_TYPE_TCP);
        aitt.Publish("test/step2/value1", dump_msg, 1600, AITT_TYPE_TCP);
        aitt.Publish("test/step2/value1", dump_msg, 1600, AITT_TYPE_TCP);

        g_timeout_add(10, AITTTCPTest::ReadyCheck, static_cast<void *>(this));

        IterateEventLoop();

        ASSERT_TRUE(ready);
    } catch (std::exception &e) {
        FAIL() << "Unexpected exception: " << e.what();
    }
}

TEST_F(AITTTCPTest, TCP_Wildcards2_Anytime)
{
    try {
        char dump_msg[204800];

        AITT aitt(clientId, MY_IP);
        aitt.Connect();

        aitt.Subscribe(
              "test/+",
              [&](aitt::MSG *handle, const void *msg, const size_t szmsg, void *cbdata) -> void {
                  AITTTCPTest *test = static_cast<AITTTCPTest *>(cbdata);
                  INFO("Got Message(Topic:%s, size:%zu)", handle->GetTopic().c_str(), szmsg);
                  static int cnt = 0;
                  ++cnt;

                  std::stringstream ss;
                  ss << "test/value" << cnt;
                  EXPECT_EQ(ss.str(), handle->GetTopic());

                  if (cnt == 3)
                      test->ToggleReady();
              },
              static_cast<void *>(this), AITT_TYPE_TCP);

        // Wait a few seconds until the AITT client gets a server list (discover devices)
        DBG("Sleep %d secs", SLEEP_MS);
        sleep(SLEEP_MS);

        aitt.Publish("test/value1", dump_msg, 12, AITT_TYPE_TCP);
        aitt.Publish("test/value2", dump_msg, 1600, AITT_TYPE_TCP);
        aitt.Publish("test/value3", dump_msg, 1600, AITT_TYPE_TCP);

        g_timeout_add(10, AITTTCPTest::ReadyCheck, static_cast<void *>(this));

        IterateEventLoop();

        ASSERT_TRUE(ready);
    } catch (std::exception &e) {
        FAIL() << "Unexpected exception: " << e.what();
    }
}
