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
package com.samsung.android.aitt;

import android.content.Context;
import android.util.Log;
import android.util.Pair;

import androidx.annotation.Nullable;

import com.google.flatbuffers.FlexBuffers;
import com.google.flatbuffers.FlexBuffersBuilder;
import com.samsung.android.modules.webrtc.WebRTC;
import com.samsung.android.modules.webrtc.WebRTCServer;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.EnumSet;
import java.util.HashMap;

public class Aitt {
    private static final String TAG = "AITT_ANDROID";
    private static final String WILL_LEAVE_NETWORK = "disconnected";
    private static final String AITT_LOCALHOST = "127.0.0.1";
    private static final int AITT_PORT = 1883;
    private static final String JAVA_SPECIFIC_DISCOVERY_TOPIC = "/java/aitt/discovery/";
    private static final String JOIN_NETWORK = "connected";
    private static final String RESPONSE_POSTFIX = "_AittRe_";
    private static final String INVALID_TOPIC = "Invalid topic";
    private static final String STATUS = "status";


    static {
        try {
            System.loadLibrary("aitt-android");
        }catch (UnsatisfiedLinkError e){
            // only ignore exception in non-android env
            if ("Dalvik".equals(System.getProperty("java.vm.name"))) throw e;
        }
    }
    private HashMap<String, ArrayList<SubscribeCallback>> subscribeCallbacks = new HashMap<>();
    private HashMap<String, HostTable> publishTable = new HashMap<>();
    private HashMap<String, Pair<Protocol , Object>> subscribeMap = new HashMap<>();
    private HashMap<String, Long> aittSubId = new HashMap<String, Long>();

    private long instance = 0;
    private String ip;
    private Context appContext;
    //ToDo - For now using sample app parameters, later fetch frameWidth & frameHeight from app
    private Integer frameWidth = 640, frameHeight = 480;

    public enum QoS {
        AT_MOST_ONCE,   // Fire and forget
        AT_LEAST_ONCE,  // Receiver is able to receive multiple times
        EXACTLY_ONCE,   // Receiver only receives exactly once
    }

    public enum Protocol {
        MQTT(0x1 << 0),    // Publish message through the MQTT
        TCP(0x1 << 1),     // Publish message to peers using the TCP
        UDP(0x1 << 2),     // Publish message to peers using the UDP
        SRTP(0x1 << 3),    // Publish message to peers using the SRTP
        WEBRTC(0x1 << 4);  // Publish message to peers using the WEBRTC

        private final int value;

        private Protocol(int value) {
            this.value = value;
        }

        public int getValue() {
            return value;
        }

        public static Protocol fromInt(long value) {
            for (Protocol type : values()) {
                if (type.getValue() == value) {
                    return type;
                }
            }
            return null;
        }
    }

    private class HostTable {
        HashMap<String, PortTable> hostMap = new HashMap<>();
    }

    private class PortTable {
        HashMap<Integer, Pair<Protocol , Object>> portMap = new HashMap<>();
    }

    public interface SubscribeCallback {
        void onMessageReceived(AittMessage message);
    }

    public Aitt(Context appContext , String id) {
        this(appContext , id, AITT_LOCALHOST, false);
    }

    public Aitt(Context appContext, String id, String ip, boolean clearSession) {
        if (appContext == null) {
            throw new IllegalArgumentException("Invalid appContext");
        }
        if (id == null || id.isEmpty()) {
            throw new IllegalArgumentException("Invalid id");
        }
        instance = initJNI(id, ip, clearSession);
        if (instance == 0L) {
            throw new RuntimeException("Failed to create native instance");
        }
        this.ip = ip;
        this.appContext = appContext;
    }

    public void connect(@Nullable String brokerIp) {
        connect(brokerIp, AITT_PORT);
    }

    public void connect(@Nullable String brokerIp, int port) {
        if (brokerIp == null || brokerIp.isEmpty()) {
            brokerIp = AITT_LOCALHOST;
        }
        connectJNI(instance, brokerIp, port);
        //Subscribe to java discovery topic
        subscribeJNI(instance, JAVA_SPECIFIC_DISCOVERY_TOPIC, Protocol.MQTT.getValue(), QoS.EXACTLY_ONCE.ordinal());
    }

    public void disconnect() {
        publishJNI(instance, JAVA_SPECIFIC_DISCOVERY_TOPIC, new byte[0], 0, Protocol.MQTT.getValue(), QoS.AT_LEAST_ONCE.ordinal(), true);

        disconnectJNI(instance);
        try {
            close();
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    public void publish(String topic, byte[] message) {
        EnumSet<Protocol> protocolSet = EnumSet.of(Protocol.MQTT);
        publish(topic, message, protocolSet, QoS.AT_MOST_ONCE, false);
    }

    public void publish(String topic, byte[] message, Protocol protocol, QoS qos, boolean retain) {
        EnumSet<Protocol> protocolSet = EnumSet.of(protocol);
        publish(topic, message, protocolSet, qos, retain);
    }

    public void publish(String topic, byte[] message, EnumSet<Protocol> protocols, QoS qos, boolean retain) {
        if (topic == null || topic.isEmpty()) {
            throw new IllegalArgumentException(INVALID_TOPIC);
        }
        if (protocols.isEmpty()) {
            throw new IllegalArgumentException("Invalid protocols");
        }
        try {
            synchronized (this) {
                if (!publishTable.containsKey(topic)) {
                    Log.e(TAG, "Invalid publish request over unsubscribed topic");
                    return;
                }
                HostTable hostTable = publishTable.get(topic);
                for (String ip : hostTable.hostMap.keySet()) {
                    PortTable portTable = hostTable.hostMap.get(ip);
                    for (Integer port : portTable.portMap.keySet()) {
                        Protocol protocol = portTable.portMap.get(port).first;
                        Object transportHandler = portTable.portMap.get(port).second;
                        if (protocol == Protocol.WEBRTC) {
                            publishWebRTC(portTable, topic, transportHandler, ip, port, message);
                        } else {
                            int proto = protocolsToInt(protocols);
                            publishJNI(instance, topic, message, message.length, proto, qos.ordinal(), retain);
                        }
                    }
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
            Log.e(TAG, "Couldnt publish to AITT C++");
        }
    }

    private void publishWebRTC(PortTable portTable, String topic, Object transportHandler, String ip, int port, byte[] message) {
        WebRTC.DataType dataType = topic.endsWith(RESPONSE_POSTFIX) ? WebRTC.DataType.Message : WebRTC.DataType.VideoFrame;
        WebRTC webrtcHandler;
        if (transportHandler == null) {
            webrtcHandler = new WebRTC(dataType, appContext);
            transportHandler = webrtcHandler;
            portTable.portMap.replace(port, new Pair<>(Protocol.WEBRTC, transportHandler));
            webrtcHandler.connect(ip, port);
        } else {
            webrtcHandler = (WebRTC) transportHandler;
        }
        if (dataType == WebRTC.DataType.Message) {
            webrtcHandler.sendMessageData(message);
        } else if (dataType == WebRTC.DataType.VideoFrame) {
            webrtcHandler.sendVideoData(message, frameWidth, frameHeight);
        }
    }

    public void subscribe(String topic, SubscribeCallback callback) {
        EnumSet<Protocol> protocolSet = EnumSet.of(Protocol.MQTT);
        subscribe(topic, callback, protocolSet, QoS.AT_MOST_ONCE);
    }

    public void subscribe(String topic, SubscribeCallback callback, Protocol protocol, QoS qos) {
        EnumSet<Protocol> protocolSet = EnumSet.of(protocol);
        subscribe(topic, callback, protocolSet, qos);
    }

    public void subscribe(String topic, SubscribeCallback callback, EnumSet<Protocol> protocols, QoS qos) {
        if (topic == null || topic.isEmpty()) {
            throw new IllegalArgumentException(INVALID_TOPIC);
        }
        if (callback == null) {
            throw new IllegalArgumentException("Invalid callback");
        }
        if (protocols.isEmpty()) {
            throw new IllegalArgumentException("Invalid protocols");
        }
        try {
            if (protocols.contains(Protocol.WEBRTC)) {
                WebRTC.ReceiveDataCallback cb = frame -> {
                    AittMessage message = new AittMessage(frame);
                    message.setTopic(topic);
                    messageReceived(message);
                };
                WebRTC.DataType dataType = topic.endsWith(RESPONSE_POSTFIX) ? WebRTC.DataType.Message : WebRTC.DataType.VideoFrame;
                WebRTCServer ws = new WebRTCServer(appContext, dataType, cb);
                int serverPort = ws.start();
                if (serverPort < 0) {
                    throw new RuntimeException("Failed to start webRTC server-socket");
                }
                synchronized (this) {
                    subscribeMap.put(topic, new Pair(Protocol.WEBRTC, ws));
                }
                byte[] data = wrapPublishData(topic, serverPort);
                publishJNI(instance, JAVA_SPECIFIC_DISCOVERY_TOPIC, data, data.length, Protocol.MQTT.value, QoS.EXACTLY_ONCE.ordinal(), true);
            } else {
                int proto = protocolsToInt(protocols);
                Long pObject = subscribeJNI(instance, topic, proto, qos.ordinal());
                aittSubId.put(topic, pObject);
            }
        } catch (Exception e) {
            e.printStackTrace();
            Log.e(TAG, "Couldnt subscribe to AITT C++");
        }
        addCallBackToSubscribeMap(topic, callback);
    }

    private byte[] wrapPublishData(String topic, int serverPort) {
        FlexBuffersBuilder fbb = new FlexBuffersBuilder(ByteBuffer.allocate(512));
        {
            int smap = fbb.startMap();
            fbb.putString(STATUS, JOIN_NETWORK);
            fbb.putString("host", this.ip);
            {
                int smap1 = fbb.startMap();
                fbb.putInt("protocol", Protocol.WEBRTC.value);
                fbb.putInt("port", serverPort);
                fbb.endMap(topic, smap1);
            }
            fbb.endMap(null, smap);
        }
        ByteBuffer buffer = fbb.finish();
        byte[] data = new byte[buffer.remaining()];
        buffer.get(data, 0, data.length);
        return data;
    }

    private void addCallBackToSubscribeMap(String topic, SubscribeCallback callback) {
        synchronized (this) {
            try {
                ArrayList<SubscribeCallback> cbList = subscribeCallbacks.get(topic);

                if (cbList != null) {
                    // check whether the list already contains same callback
                    if (!cbList.contains(callback)) {
                        cbList.add(callback);
                    }
                } else {
                    cbList = new ArrayList<>();
                    cbList.add(callback);
                    subscribeCallbacks.put(topic, cbList);
                }
            } catch (Exception e) {
                e.printStackTrace();
                Log.e(TAG, "Couldnt subscribe to AITT C++");
            }
        }
    }

    public void unsubscribe(String topic) {
        if (topic == null || topic.isEmpty()) {
            throw new IllegalArgumentException(INVALID_TOPIC);
        }

        boolean isRemoved = false;
        try {
            synchronized (this) {
                if (subscribeMap.containsKey(topic) && subscribeMap.get(topic).first == Protocol.WEBRTC) {
                    WebRTCServer ws = (WebRTCServer) subscribeMap.get(topic).second;
                    ws.stop();
                    subscribeMap.remove(topic);
                    isRemoved = true;
                }
            }

            if (!isRemoved) {
                Long paittSubId = null;
                synchronized (this) {
                    if (aittSubId.containsKey(topic)) {
                        paittSubId = aittSubId.get(topic);
                    }
                }
                if (paittSubId != null) {
                    unsubscribeJNI(instance, paittSubId);
                }
            }

            synchronized (this) {
                subscribeCallbacks.remove(topic);
                aittSubId.remove(topic);
            }
        } catch (Exception e) {
            e.printStackTrace();
            Log.e(TAG, "Exception during un subscribe to AITT C++");
        }
    }

    private void messageCallback(String topic, byte[] payload) {
        try {
            if (topic.compareTo(JAVA_SPECIFIC_DISCOVERY_TOPIC) == 0) {
                if (payload.length <= 0) {
                    Log.e(TAG, "Invlaid payload, Ignore");
                    return;
                }
                discoveryMessageCallback(payload);
            } else {
                AittMessage message = new AittMessage(payload);
                message.setTopic(topic);
                messageReceived(message);
            }
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    private void discoveryMessageCallback(byte[] payload) {
        /*
           Flexbuffer discovery message expected
          {
            "status": "connected",
            "host": "127.0.0.1",
            "/customTopic/aitt/faceRecog": {
               "protocol": 1,
               "port": 108081,
            },
            "/customTopic/aitt/ASR": {
               "protocol": 2,
               "port": 102020,
            },

            ...

             "/customTopic/aitt/+": {
               "protocol": 3,
               "port": 20123,
            },
           }
        */
        try {
            ByteBuffer buffer = ByteBuffer.wrap(payload);
            FlexBuffers.Map map = FlexBuffers.getRoot(buffer).asMap();
            String host = map.get("host").asString();
            String status = map.get(STATUS).asString();
            if (status != null && status.compareTo(WILL_LEAVE_NETWORK) == 0) {
                synchronized (this) {
                    for (String _topic : publishTable.keySet()) {
                        HostTable hostTable = publishTable.get(_topic);
                        if (hostTable != null) {
                            hostTable.hostMap.remove(host);
                        }
                    }
                }
                return;
            }

            FlexBuffers.KeyVector topics = map.keys();
            for (int i = 0; i < topics.size(); i++) {
                String _topic = topics.get(i).toString();
                if (_topic.compareTo("host") == 0 || _topic.compareTo(STATUS) == 0) {
                    continue;
                }

                FlexBuffers.Map _map = map.get(_topic).asMap();
                int port = _map.get("port").asInt();
                long p = _map.get("protocol").asUInt();
                Protocol protocol = Protocol.fromInt(p);
                updatePublishTable(_topic, host, port, protocol);
            }
        } catch (Exception e) {
            e.printStackTrace();
            Log.e(TAG, "Couldnt un subscribe to AITT C++");
        }
    }

    private void updatePublishTable(String topic, String host, int port, Protocol protocol) {
        synchronized(this) {
            if (!publishTable.containsKey(topic)) {
                PortTable portTable = new PortTable();
                portTable.portMap.put(port, new Pair(protocol , null));
                HostTable hostTable = new HostTable();
                hostTable.hostMap.put(host, portTable);
                publishTable.put(topic, hostTable);
                return;
            }

            HostTable hostTable = publishTable.get(topic);
            if (!hostTable.hostMap.containsKey(host)) {
                PortTable portTable = new PortTable();
                portTable.portMap.put(port, new Pair(protocol , null));
                hostTable.hostMap.put(host, portTable);
                return;
            }

            PortTable portTable = hostTable.hostMap.get(host);
            if (portTable.portMap.containsKey(port)) {
                portTable.portMap.replace(port, new Pair(protocol , null));
                return;
            }

            portTable.portMap.put(port, new Pair(protocol , null));
        }
    }

    private void messageReceived(AittMessage message) {
        try {

            String topic = message.getTopic();
            synchronized (this) {
                ArrayList<SubscribeCallback> cbList = subscribeCallbacks.get(topic);

                if (cbList != null) {
                    for (int i = 0; i < cbList.size(); i++) {
                        cbList.get(i).onMessageReceived(message);
                    }
                }
            }
        } catch (Exception e) {
            e.printStackTrace();
            Log.e(TAG, "Exception in messageReceived");
        }
    }

    private int protocolsToInt(EnumSet<Protocol> protocols) {
        int proto = 0;
        for (Protocol p : Protocol.values()) {
            if (protocols.contains(p)) {
                proto += p.getValue();
            }
        }
        return proto;
    }

    public void close() {
        synchronized (this) {
            if(subscribeCallbacks!=null) {
                subscribeCallbacks.clear();
                subscribeCallbacks = null;
            }
            if(aittSubId!=null) {
                aittSubId.clear();
                aittSubId = null;
            }
        }
    }

    /* native API's set */
    /* API to initialize JNI */
    private native long initJNI(String id, String ip, boolean clearSession);

    /* API's for Discovery using MQTT */
    private native void connectJNI(long instance, final String host, int port);

    private native void disconnectJNI(long instance);

    private native void publishJNI(long instance, final String topic, final byte[] data, long datalen, int protocol, int qos, boolean retain);

    private native long subscribeJNI(long instance, final String topic, int protocol, int qos);

    private native void unsubscribeJNI(long instance, final long aittSubId);
}
