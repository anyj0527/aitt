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
package com.samsung.android.aittnative;

/**
 * Jni Interface class as intermediate layer for android aitt and other transport modules to interact with JNI module.
 */
public class JniInterface {

    private JniCallback jniCallback;
    private JniConnectionCallback jniConnectionCallback;
    private long instance = 0;

    /**
     * Load aitt-android library
     */
    static {
        try {
            System.loadLibrary("aitt-native");
        } catch (UnsatisfiedLinkError e) {
            // only ignore exception in non-android env
            if ("Dalvik".equals(System.getProperty("java.vm.name"))) throw e;
        }
    }

    /**
     * JNI callback interface to send data from JNI layer to Java layer(aitt or transport module)
     */
    public interface JniCallback {
        void jniDataPush(String topic, byte[] data);
    }

    /**
     * JNI callback interface to send connection callback status to aitt layer
     */
    public interface JniConnectionCallback {
        void jniConnectionCB(int status);
    }

    /**
     * JNI interface constructor
     */
    public JniInterface() {

    }

    /**
     * JNI interface API to initialize JNI module
     * @param id unique mqtt id
     * @param ip self IP address of device
     * @param clearSession to clear current session if client disconnects
     * @return returns the JNI instance object in long
     */
    public long init(String id, String ip, boolean clearSession) {
        instance = initJNI(id, ip, clearSession);
        return instance;
    }

    /**
     * JNI Interface API to connect to MQTT broker
     * @param brokerIp mqtt broker ip address
     * @param port mqtt broker port number
     */
    public void connect(String brokerIp, int port) {
        connectJNI(instance, brokerIp, port);
    }

    /**
     * JNI Interface API to subscribe to a topic
     * @param topic String to which applications can subscribe, to receive data
     * @param protocol Protocol supported by application, invoking subscribe
     * @param qos QoS at which the message should be delivered
     * @return returns the subscribe instance in long
     */
    public long subscribe(final String topic, int protocol, int qos) {
        return subscribeJNI(instance, topic, protocol, qos);
    }

    /**
     * JNI Interface API to disconnect from broker
     */
    public void disconnect() {
        disconnectJNI(instance);
    }

    /**
     * JNI Interface API to publish data to specified topic
     * @param topic String to which message needs to be published
     * @param data Byte message to be published
     * @param datalen Size/length of the message to be published
     * @param protocol Protocol to be used to publish message
     * @param qos QoS at which the message should be delivered
     * @param retain Boolean to decide whether or not the message should be retained by the broker
     */
    public void publish(final String topic, final byte[] data, long datalen, int protocol, int qos, boolean retain) {
        publishJNI(instance, topic, data, datalen, protocol, qos, retain);
    }

    /**
     * JNI Interface API to unsubscribe the given topic
     * @param aittSubId Subscribe ID of the topics to be unsubscribed
     */
    public void unsubscribe(final long aittSubId) {
        unsubscribeJNI(instance, aittSubId);
    }

    /**
     * JNI Interface API to set connection callback instance
     * @param cb callback instance of JniConnectionCallback interface
     */
    public void setConnectionCallback(JniConnectionCallback cb) {
        jniConnectionCallback = cb;
        setConnectionCallbackJNI(instance);
    }

    /**
     * JNI Interface API to register jni callback instance
     * @param callBack callback instance of JniCallback interface
     */
    public void registerJniCallback(JniCallback callBack) {
        jniCallback = callBack;
    }

    /**
     * messageCallback API to receive data from JNI layer to JNI interface layer
     * @param topic Topic to which data is received
     * @param payload Data that is sent from JNI to JNI interface layer
     */
    private void messageCallback(String topic, byte[] payload) {
        jniCallback.jniDataPush(topic, payload);
    }

    /**
     * connectionStatusCallback API to receive connection status from JNI to JNI interface layer
     * @param status status of the device connection with mqtt broker
     */
    private void connectionStatusCallback(int status) {
        if (jniConnectionCallback != null) {
            jniConnectionCallback.jniConnectionCB(status);
        }
    }

    /* native API's set */
    /* Native API to initialize JNI */
    private native long initJNI(String id, String ip, boolean clearSession);

    /* Native API for connecting to broker */
    private native void connectJNI(long instance, final String host, int port);

    /* Native API for disconnecting from broker */
    private native void disconnectJNI(long instance);

    /* Native API for setting connection callback */
    private native void setConnectionCallbackJNI(long instance);

    /* Native API for publishing to a topic */
    private native void publishJNI(long instance, final String topic, final byte[] data, long datalen, int protocol, int qos, boolean retain);

    /* Native API for subscribing to a topic */
    private native long subscribeJNI(long instance, final String topic, int protocol, int qos);

    /* Native API for unsubscribing a topic */
    private native void unsubscribeJNI(long instance, final long aittSubId);
}
