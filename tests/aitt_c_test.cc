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
#include "aitt_c.h"

#include <glib.h>
#include <gtest/gtest.h>

#include "AittTests.h"
#include "aitt_internal.h"

TEST(AITT_C_INTERFACE, new_P_Anytime)
{
    aitt_option_h option = aitt_option_new();
    ASSERT_NE(option, nullptr);

    aitt_h handle = aitt_new("test1", option);
    aitt_option_destroy(option);
    EXPECT_TRUE(handle != nullptr);
    aitt_destroy(handle);

    handle = aitt_new(nullptr, nullptr);
    EXPECT_TRUE(handle != nullptr);
    aitt_destroy(handle);

    handle = aitt_new("", nullptr);
    EXPECT_TRUE(handle != nullptr);
    aitt_destroy(handle);
}

TEST(AITT_C_INTERFACE, destroy_P_Anytime)
{
    aitt_h handle = aitt_new("test2", nullptr);
    ASSERT_NE(handle, nullptr);

    aitt_destroy(handle);
    aitt_destroy(nullptr);
}

TEST(AITT_C_INTERFACE, option_P_Anytime)
{
    int ret;

    aitt_option_h option = aitt_option_new();
    ASSERT_NE(option, nullptr);

    ret = aitt_option_set(option, AITT_OPT_MY_IP, LOCAL_IP);
    EXPECT_EQ(ret, AITT_ERROR_NONE);
    EXPECT_STREQ(LOCAL_IP, aitt_option_get(option, AITT_OPT_MY_IP));

    ret = aitt_option_set(option, AITT_OPT_MY_IP, nullptr);
    EXPECT_EQ(ret, AITT_ERROR_NONE);
    EXPECT_STREQ(nullptr, aitt_option_get(option, AITT_OPT_MY_IP));

    ret = aitt_option_set(option, AITT_OPT_CLEAN_SESSION, "TRUE");
    EXPECT_EQ(ret, AITT_ERROR_NONE);
    EXPECT_STREQ("true", aitt_option_get(option, AITT_OPT_CLEAN_SESSION));

    ret = aitt_option_set(option, AITT_OPT_CLEAN_SESSION, nullptr);
    EXPECT_EQ(ret, AITT_ERROR_NONE);
    EXPECT_STREQ("false", aitt_option_get(option, AITT_OPT_CLEAN_SESSION));

    ret = aitt_option_set(option, AITT_OPT_CUSTOM_BROKER, "TRUE");
    EXPECT_EQ(ret, AITT_ERROR_NONE);
    EXPECT_STREQ("true", aitt_option_get(option, AITT_OPT_CUSTOM_BROKER));

    ret = aitt_option_set(option, AITT_OPT_CUSTOM_BROKER, nullptr);
    EXPECT_EQ(ret, AITT_ERROR_NONE);
    EXPECT_STREQ("false", aitt_option_get(option, AITT_OPT_CUSTOM_BROKER));

    aitt_option_destroy(option);
}

TEST(AITT_C_INTERFACE, option_N_Anytime)
{
    int ret;

    aitt_option_h option = aitt_option_new();
    ASSERT_NE(option, nullptr);

    ret = aitt_option_set(option, AITT_OPT_UNKNOWN, LOCAL_IP);
    EXPECT_EQ(ret, AITT_ERROR_INVALID_PARAMETER);

    ret = aitt_option_set(option, AITT_OPT_CLEAN_SESSION, "OFF");
    EXPECT_EQ(ret, AITT_ERROR_INVALID_PARAMETER);

    ret = aitt_option_set(option, AITT_OPT_CUSTOM_BROKER, "Off");
    EXPECT_EQ(ret, AITT_ERROR_INVALID_PARAMETER);

    ret = aitt_option_set(nullptr, AITT_OPT_MY_IP, LOCAL_IP);
    EXPECT_EQ(ret, AITT_ERROR_INVALID_PARAMETER);

    aitt_option_destroy(option);
}

TEST(AITT_C_INTERFACE, connect_disconnect_P_Anytime)
{
    int ret;

    aitt_option_h option = aitt_option_new();
    ASSERT_NE(option, nullptr);

    ret = aitt_option_set(option, AITT_OPT_MY_IP, LOCAL_IP);
    EXPECT_EQ(ret, AITT_ERROR_NONE);

    aitt_h handle = aitt_new("test5", option);
    aitt_option_destroy(option);
    ASSERT_NE(handle, nullptr);

    ret = aitt_connect(handle, LOCAL_IP, 1883);
    ASSERT_EQ(ret, AITT_ERROR_NONE);

    ret = aitt_disconnect(handle);
    EXPECT_EQ(ret, AITT_ERROR_NONE);

    aitt_destroy(handle);
}

TEST(AITT_C_INTERFACE, connect_N_Anytime)
{
    int ret;

    aitt_h handle = aitt_new("test6", nullptr);
    ASSERT_NE(handle, nullptr);

    aitt_h invalid_handle = nullptr;
    ret = aitt_connect(invalid_handle, LOCAL_IP, 1883);
    EXPECT_EQ(ret, AITT_ERROR_INVALID_PARAMETER);

    const char *invalid_ip = "1.2.3";
    ret = aitt_connect(handle, invalid_ip, 1883);
    EXPECT_EQ(ret, AITT_ERROR_INVALID_PARAMETER);

    invalid_ip = "";
    ret = aitt_connect(handle, invalid_ip, 1883);
    EXPECT_EQ(ret, AITT_ERROR_INVALID_PARAMETER);

    int invalid_port = -1;
    ret = aitt_connect(handle, LOCAL_IP, invalid_port);
    EXPECT_EQ(ret, AITT_ERROR_SYSTEM);

    aitt_destroy(handle);
}

TEST(AITT_C_INTERFACE, disconnect_N_Anytime)
{
    int ret;

    ret = aitt_disconnect(nullptr);
    EXPECT_EQ(ret, AITT_ERROR_INVALID_PARAMETER);

    aitt_h handle = aitt_new("test7", nullptr);
    ASSERT_NE(handle, nullptr);

    ret = aitt_disconnect(handle);
    EXPECT_EQ(ret, AITT_ERROR_NOT_READY);

    aitt_destroy(handle);
}

TEST(AITT_C_INTERFACE, pub_sub_P_Anytime)
{
    int ret;

    aitt_option_h option = aitt_option_new();
    ASSERT_NE(option, nullptr);

    ret = aitt_option_set(option, AITT_OPT_MY_IP, LOCAL_IP);
    EXPECT_EQ(ret, AITT_ERROR_NONE);

    aitt_h handle = aitt_new("test8", option);
    aitt_option_destroy(option);
    ASSERT_NE(handle, nullptr);

    ret = aitt_connect(handle, LOCAL_IP, 1883);
    ASSERT_EQ(ret, AITT_ERROR_NONE);

    GMainLoop *loop = g_main_loop_new(nullptr, FALSE);
    aitt_sub_h sub_handle = nullptr;
    ret = aitt_subscribe(
          handle, TEST_C_TOPIC,
          [](aitt_msg_h msg_handle, const void *msg, size_t msg_len, void *user_data) {
              GMainLoop *loop = static_cast<GMainLoop *>(user_data);
              std::string received_data((const char *)msg, msg_len);
              EXPECT_STREQ(received_data.c_str(), TEST_C_MSG);
              EXPECT_STREQ(aitt_msg_get_topic(msg_handle), TEST_C_TOPIC);
              g_main_loop_quit(loop);
          },
          loop, &sub_handle);
    ASSERT_EQ(ret, AITT_ERROR_NONE);
    EXPECT_TRUE(sub_handle != nullptr);

    ret = aitt_publish(handle, TEST_C_TOPIC, TEST_C_MSG, strlen(TEST_C_MSG));
    ASSERT_EQ(ret, AITT_ERROR_NONE);

    g_main_loop_run(loop);
    g_main_loop_unref(loop);

    ret = aitt_disconnect(handle);
    EXPECT_EQ(ret, AITT_ERROR_NONE);

    aitt_destroy(handle);
}

TEST(AITT_C_INTERFACE, pub_N_Anytime)
{
    int ret;

    aitt_option_h option = aitt_option_new();
    ASSERT_NE(option, nullptr);

    ret = aitt_option_set(option, AITT_OPT_MY_IP, LOCAL_IP);
    EXPECT_EQ(ret, AITT_ERROR_NONE);

    aitt_h handle = aitt_new("test9", option);
    aitt_option_destroy(option);
    ASSERT_NE(handle, nullptr);

    ret = aitt_connect(handle, nullptr, 1883);
    EXPECT_NE(ret, AITT_ERROR_NONE);

    ret = aitt_publish(handle, TEST_C_TOPIC, TEST_C_MSG, strlen(TEST_C_MSG));
    EXPECT_EQ(ret, AITT_ERROR_NOT_READY);

    ret = aitt_publish(nullptr, TEST_C_TOPIC, TEST_C_MSG, strlen(TEST_C_MSG));
    EXPECT_EQ(ret, AITT_ERROR_INVALID_PARAMETER);

    ret = aitt_connect(handle, LOCAL_IP, 1883);
    ASSERT_EQ(ret, AITT_ERROR_NONE);

    ret = aitt_publish(handle, nullptr, TEST_C_MSG, strlen(TEST_C_MSG));
    EXPECT_EQ(ret, AITT_ERROR_INVALID_PARAMETER);

    ret = aitt_publish(handle, TEST_C_TOPIC, nullptr, strlen(TEST_C_MSG));
    EXPECT_EQ(ret, AITT_ERROR_INVALID_PARAMETER);

    aitt_disconnect(handle);
    aitt_destroy(handle);
}

TEST(AITT_C_INTERFACE, sub_N_Anytime)
{
    int ret;

    aitt_option_h option = aitt_option_new();
    ASSERT_NE(option, nullptr);

    ret = aitt_option_set(option, AITT_OPT_MY_IP, LOCAL_IP);
    EXPECT_EQ(ret, AITT_ERROR_NONE);

    aitt_h handle = aitt_new("test10", option);
    aitt_option_destroy(option);
    aitt_sub_h sub_handle = nullptr;
    ASSERT_NE(handle, nullptr);

    ret = aitt_connect(handle, nullptr, 1883);
    EXPECT_NE(ret, AITT_ERROR_NONE);

    ret = aitt_subscribe(
          handle, TEST_C_TOPIC, [](aitt_msg_h, const void *, size_t, void *) {}, nullptr,
          &sub_handle);
    EXPECT_EQ(ret, AITT_ERROR_NOT_READY);

    ret = aitt_subscribe(
          nullptr, TEST_C_TOPIC, [](aitt_msg_h, const void *, size_t, void *) {}, nullptr,
          &sub_handle);
    EXPECT_EQ(ret, AITT_ERROR_INVALID_PARAMETER);

    ret = aitt_connect(handle, LOCAL_IP, 1883);
    ASSERT_EQ(ret, AITT_ERROR_NONE);

    ret = aitt_subscribe(
          handle, nullptr, [](aitt_msg_h, const void *, size_t, void *) {}, nullptr, &sub_handle);
    EXPECT_EQ(ret, AITT_ERROR_INVALID_PARAMETER);

    ret = aitt_subscribe(handle, TEST_C_TOPIC, nullptr, nullptr, &sub_handle);
    EXPECT_EQ(ret, AITT_ERROR_INVALID_PARAMETER);

    aitt_disconnect(handle);
    aitt_destroy(handle);
}

#define reply_msg "hello reply message"
#define test_correlation "0001"

TEST(AITT_C_INTERFACE, pub_with_reply_send_reply_P_Anytime)
{
    int ret;

    aitt_option_h option = aitt_option_new();
    ASSERT_NE(option, nullptr);

    ret = aitt_option_set(option, AITT_OPT_MY_IP, LOCAL_IP);
    EXPECT_EQ(ret, AITT_ERROR_NONE);

    aitt_h handle = aitt_new("test11", option);
    aitt_option_destroy(option);
    ASSERT_NE(handle, nullptr);

    ret = aitt_connect(handle, LOCAL_IP, 1883);
    ASSERT_EQ(ret, AITT_ERROR_NONE);

    GMainLoop *loop = g_main_loop_new(nullptr, FALSE);
    aitt_sub_h sub_handle = nullptr;
    ret = aitt_subscribe(
          handle, TEST_C_TOPIC,
          [](aitt_msg_h msg_handle, const void *msg, size_t msg_len, void *user_data) {
              aitt_h handle = static_cast<aitt_h>(user_data);
              std::string received_data((const char *)msg, msg_len);
              EXPECT_STREQ(received_data.c_str(), TEST_C_MSG);
              EXPECT_STREQ(aitt_msg_get_topic(msg_handle), TEST_C_TOPIC);
              aitt_send_reply(handle, msg_handle, reply_msg, sizeof(reply_msg), true);
          },
          handle, &sub_handle);
    ASSERT_EQ(ret, AITT_ERROR_NONE);

    ret = aitt_publish_with_reply(
          handle, TEST_C_TOPIC, TEST_C_MSG, strlen(TEST_C_MSG), AITT_TYPE_MQTT,
          AITT_QOS_AT_MOST_ONCE, test_correlation,
          [](aitt_msg_h msg_handle, const void *msg, size_t msg_len, void *user_data) {
              GMainLoop *loop = static_cast<GMainLoop *>(user_data);
              std::string received_data((const char *)msg, msg_len);
              EXPECT_STREQ(received_data.c_str(), reply_msg);
              g_main_loop_quit(loop);
          },
          loop);

    g_main_loop_run(loop);
    g_main_loop_unref(loop);

    ret = aitt_disconnect(handle);
    EXPECT_EQ(ret, AITT_ERROR_NONE);

    aitt_destroy(handle);
}

TEST(AITT_C_INTERFACE, pub_with_reply_N_Anytime)
{
    int ret;

    aitt_option_h option = aitt_option_new();
    ASSERT_NE(option, nullptr);

    ret = aitt_option_set(option, AITT_OPT_MY_IP, LOCAL_IP);
    EXPECT_EQ(ret, AITT_ERROR_NONE);

    aitt_h handle = aitt_new("test12", option);
    aitt_option_destroy(option);
    ASSERT_NE(handle, nullptr);

    ret = aitt_connect(handle, LOCAL_IP, 1883);
    ASSERT_EQ(ret, AITT_ERROR_NONE);

    ret = aitt_publish_with_reply(
          nullptr, TEST_C_TOPIC, TEST_C_MSG, strlen(TEST_C_MSG), AITT_TYPE_MQTT,
          AITT_QOS_AT_MOST_ONCE, test_correlation,
          [](aitt_msg_h msg_handle, const void *msg, size_t msg_len, void *user_data) {}, nullptr);
    EXPECT_EQ(ret, AITT_ERROR_INVALID_PARAMETER);

    ret = aitt_publish_with_reply(
          handle, nullptr, TEST_C_MSG, strlen(TEST_C_MSG), AITT_TYPE_MQTT, AITT_QOS_AT_MOST_ONCE,
          test_correlation,
          [](aitt_msg_h msg_handle, const void *msg, size_t msg_len, void *user_data) {}, nullptr);
    EXPECT_EQ(ret, AITT_ERROR_INVALID_PARAMETER);

    ret = aitt_publish_with_reply(
          handle, TEST_C_TOPIC, nullptr, 0, AITT_TYPE_MQTT, AITT_QOS_AT_MOST_ONCE, test_correlation,
          [](aitt_msg_h msg_handle, const void *msg, size_t msg_len, void *user_data) {}, nullptr);
    EXPECT_EQ(ret, AITT_ERROR_INVALID_PARAMETER);

    ret = aitt_publish_with_reply(handle, TEST_C_TOPIC, TEST_C_MSG, strlen(TEST_C_MSG),
          AITT_TYPE_MQTT, AITT_QOS_AT_MOST_ONCE, test_correlation, nullptr, nullptr);
    EXPECT_EQ(ret, AITT_ERROR_INVALID_PARAMETER);

    aitt_destroy(handle);
}

TEST(AITT_C_INTERFACE, sub_unsub_P_Anytime)
{
    int ret;

    aitt_option_h option = aitt_option_new();
    ASSERT_NE(option, nullptr);

    ret = aitt_option_set(option, AITT_OPT_MY_IP, LOCAL_IP);
    EXPECT_EQ(ret, AITT_ERROR_NONE);

    aitt_h handle = aitt_new("test13", option);
    aitt_option_destroy(option);
    ASSERT_NE(handle, nullptr);

    ret = aitt_connect(handle, LOCAL_IP, 1883);
    ASSERT_EQ(ret, AITT_ERROR_NONE);

    static unsigned int sub_call_count = 0;
    GMainLoop *loop = g_main_loop_new(nullptr, FALSE);
    static aitt_sub_h sub_handle = nullptr;
    ret = aitt_subscribe(
          handle, TEST_C_TOPIC,
          [](aitt_msg_h msg_handle, const void *msg, size_t msg_len, void *user_data) {
              sub_call_count++;
          },
          nullptr, &sub_handle);
    ASSERT_EQ(ret, AITT_ERROR_NONE);
    EXPECT_TRUE(sub_handle != nullptr);

    ret = aitt_publish(handle, TEST_C_TOPIC, TEST_C_MSG, strlen(TEST_C_MSG));
    ASSERT_EQ(ret, AITT_ERROR_NONE);

    g_timeout_add(
          1000,
          [](gpointer data) -> gboolean {
              aitt_h handle = static_cast<aitt_h>(data);
              int ret = aitt_unsubscribe(handle, sub_handle);
              EXPECT_EQ(ret, AITT_ERROR_NONE);
              sub_handle = nullptr;

              ret = aitt_publish(handle, TEST_C_TOPIC, TEST_C_MSG, strlen(TEST_C_MSG));
              EXPECT_EQ(ret, AITT_ERROR_NONE);
              return FALSE;
          },
          handle);

    g_timeout_add(
          2000,
          [](gpointer data) -> gboolean {
              GMainLoop *loop = static_cast<GMainLoop *>(data);
              EXPECT_EQ(sub_call_count, 1);

              if (sub_call_count == 1) {
                  g_main_loop_quit(loop);
                  return FALSE;
              }

              return TRUE;
          },
          loop);

    g_main_loop_run(loop);
    g_main_loop_unref(loop);

    ret = aitt_disconnect(handle);
    EXPECT_EQ(ret, AITT_ERROR_NONE);

    aitt_destroy(handle);
}

TEST(AITT_C_INTERFACE, will_set_N_Anytime)
{
    int ret;

    ret = aitt_will_set(nullptr, "test/will_topic", "test", 4, AITT_QOS_AT_MOST_ONCE, false);
    EXPECT_EQ(ret, AITT_ERROR_INVALID_PARAMETER);
}
