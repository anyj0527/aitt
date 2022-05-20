/*
 * Copyright (c) 2021 Samsung Electronics Co., Ltd All Rights Reserved
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

package com.samsung.android.aitt;

import android.content.Context;

import org.robolectric.annotation.Implementation;
import org.robolectric.annotation.Implements;

@Implements(Aitt.class)
public class ShadowAitt {
    private static ShadowAitt sInstance;

    @Implementation
    public void __constructor__(Context appContext, String id, String ip, boolean clearSession){
        sInstance =this;
        if (appContext == null) {
            throw new IllegalArgumentException("Invalid appContext");
        }
        if (id == null || id.isEmpty()) {
            throw new IllegalArgumentException("Invalid id");
        }
    }

    @Implementation
    long initJNI(String id, String ip, boolean clearSession){
        return 1L;
    }

    @Implementation
    void connectJNI(long instance, final String host, int port){
    }

    @Implementation
    void publishJNI(long instance, final String topic, final byte[] data, long datalen, int protocol, int qos, boolean retain){
    }

    @Implementation
    void disconnectJNI(long instance){

    }
    @Implementation
    long subscribeJNI(long instance, final String topic, int protocol, int qos){
        return 1L;
    }

    @Implementation
    void unsubscribeJNI(long instance, final long aittSubId){
    }

}
