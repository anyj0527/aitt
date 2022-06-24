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
#include "AITT.h"

#include <glib.h>
#include <gtest/gtest.h>

#include "aitt_internal.h"

#define TEST_MSG "This is my test message"
#define LOCAL_IP "127.0.0.1"

using AITT = aitt::AITT;

class AITTManualTest : public testing::Test {
  public:
    void ToggleReady() { ready = true; }

    bool ready;

  protected:
    void SetUp() override
    {
        ready = false;
        mainLoop = g_main_loop_new(nullptr, FALSE);
    }

    void IterateEventLoop(void)
    {
        g_main_loop_run(mainLoop);
        DBG("Go forward");
    }

    void TearDown() override { g_main_loop_unref(mainLoop); }

    static gboolean ReadyCheck(gpointer data)
    {
        AITTManualTest *test = static_cast<AITTManualTest *>(data);

        if (test->ready) {
            g_main_loop_quit(test->mainLoop);
            return FALSE;
        }

        return TRUE;
    }

    GMainLoop *mainLoop;
};

TEST_F(AITTManualTest, WillSet_P)
{
    try {
        AITT aitt("", LOCAL_IP, true);
        aitt.Connect();
        aitt.Subscribe(
              "test/AITT_will",
              [](aitt::MSG *handle, const void *msg, const size_t szmsg, void *cbdata) -> void {
                  AITTManualTest *test = static_cast<AITTManualTest *>(cbdata);
                  test->ToggleReady();
                  DBG("Subscribe invoked: %s %zu", static_cast<const char *>(msg), szmsg);
              },
              static_cast<void *>(this));

        int pid = fork();
        if (pid == 0) {
            AITT aitt_will("test_will_AITT", LOCAL_IP, true);
            aitt_will.SetWillInfo("test/AITT_will", TEST_MSG, sizeof(TEST_MSG),
                  AITT_QOS_AT_LEAST_ONCE, false);
            aitt_will.Connect();
            sleep(2);
            // Do not call aitt_will.Disconnect()
        } else {
            sleep(1);
            kill(pid, SIGKILL);

            g_timeout_add(10, AITTManualTest::ReadyCheck, static_cast<void *>(this));
            IterateEventLoop();

            ASSERT_TRUE(ready);
        }
    } catch (std::exception &e) {
        FAIL() << "Unexpected exception: " << e.what();
    }
}

TEST(AITT_MANUAL, Connect_with_ID_P)
{
    try {
        AITT aitt("", LOCAL_IP);
        aitt.Connect(LOCAL_IP, 1883, "testID", "testPasswd");
    } catch (std::exception &e) {
        FAIL() << "Unexpected exception: " << e.what();
    }
}
