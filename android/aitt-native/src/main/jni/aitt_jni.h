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
#include <android/log.h>
#include <jni.h>
#include <string>

#define TAG "AITT_ANDROID_JNI"
#define JNI_LOG(a, b, c) __android_log_write(a, b, c)

using AITT = aitt::AITT;

class AittNativeInterface {
private:
    struct CallbackContext {
        JavaVM *jvm;
        jmethodID messageCallbackMethodID;
        jmethodID connectionCallbackMethodID;
    };

private:
    AittNativeInterface(std::string &mqId, std::string &ip, bool clearSession);

    virtual ~AittNativeInterface(void);

    static std::string GetStringUTF(JNIEnv *env, jstring str);

    static bool checkParams(JNIEnv *env, jobject jniInterfaceObject);

    static bool jniStatusCheck(JNIEnv *&env, int JNIStatus);

public:
    static jlong init(JNIEnv *env, jobject jniInterfaceObject,
                      jstring id, jstring ip, jboolean clearSession);

    static void connect(JNIEnv *env, jobject jniInterfaceObject, jlong handle,
                        jstring host, jint port);

    static jlong subscribe(JNIEnv *env, jobject jniInterfaceObject, jlong handle,
                           jstring topic, jint protocol, jint qos);

    static void publish(JNIEnv *env, jobject jniInterfaceObject, jlong handle,
                        jstring topic, jbyteArray data, jlong datalen, jint protocol,
                        jint qos, jboolean retain);

    static void unsubscribe(JNIEnv *env, jobject jniInterfaceObject, jlong handle,
                            jlong aittSubId);

    static void disconnect(JNIEnv *env, jobject jniInterfaceObject, jlong handle);

    static void setConnectionCallback(JNIEnv *env, jobject jniInterfaceObject, jlong handle);

private:
    AITT aitt;
    jobject cbObject;
    static CallbackContext cbContext;
};
