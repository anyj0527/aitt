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

import static org.junit.Assert.assertNotNull;

import android.content.Context;
import android.util.Log;

import androidx.test.core.app.ApplicationProvider;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.robolectric.RobolectricTestRunner;
import org.robolectric.annotation.Config;

import java.util.EnumSet;
@RunWith(RobolectricTestRunner.class)
@Config(shadows = ShadowAitt.class)
public class AittUnitTest {
   private static final String TAG = "AITTUnitTest";
   private String id = "id101";
   private String ip = "127.0.0.1";
   @Mock
   private Context appContext = ApplicationProvider.getApplicationContext();
   private String brokerIp = "192.168.0.1";
   private int port = 1803;
   private String topic = "aitt/test";
   private String message = "test message";

   @Test
   public void testInitialize() {
      Aitt aitt = null;
      try {
         aitt = new Aitt(appContext, id, ip, false);
      } catch (InstantiationException e) {
         Log.e(TAG, "Error during testInitialize", e);
      }
      assertNotNull(aitt);
      aitt.close();
   }

   @Test
   public void testInitializeOnlyId() {
      Aitt aitt = null;
      try {
         aitt = new Aitt(appContext, id);
         assertNotNull(aitt);
         aitt.close();
      } catch (InstantiationException e) {
         Log.e(TAG, "Error during testInitializeOnlyId", e);
      }
   }

   @Test(expected = IllegalArgumentException.class)
   public void testInitializeInvalidId() {
      String _id = "";
      Aitt aitt = null;
      try {
         aitt = new Aitt(appContext, _id);
         aitt.close();
      } catch (InstantiationException e) {
         Log.e(TAG, "Error during testInitializeInvalidId", e);
      }
   }

   @Test(expected = IllegalArgumentException.class)
   public void testInitializeInvalidContext() {
      String _id = "";
      Aitt aitt = null;
      try {
         aitt = new Aitt(null, _id);
         aitt.close();
      } catch (InstantiationException e) {
         Log.e(TAG, "Error during testInitializeInvalidContext", e);
      }
   }

   @Test
   public void testConnect() {
      Aitt aitt = null;
      try {
         aitt = new Aitt(appContext, id);
      } catch (InstantiationException e) {
         Log.e(TAG, "Error during testConnect", e);
      }
      assertNotNull(aitt);
      aitt.connect(brokerIp, port);
      aitt.close();
   }

   @Test
   public void testConnectWithoutIP() {
      Aitt aitt = null;
      try {
         aitt = new Aitt(appContext, id);
      } catch (InstantiationException e) {
         Log.e(TAG, "Error during testConnectWithoutIP", e);
      }
      assertNotNull(aitt);
      aitt.connect(null);
      aitt.close();
   }

   @Test
   public void testDisconnect() {
      Aitt aitt = null;
      try {
         aitt = new Aitt(appContext, id);
      } catch (InstantiationException e) {
         Log.e(TAG, "Error during testDisconnect", e);
      }
      assertNotNull(aitt);
      aitt.connect(brokerIp, port);
      aitt.disconnect();
      aitt.close();
   }

   @Test
   public void testPublishMqtt() {
      Aitt aitt = null;
      try {
         aitt = new Aitt(appContext, id);
      } catch (InstantiationException e) {
         Log.e(TAG, "Error during testPublishMqtt", e);
      }
      assertNotNull(aitt);
      aitt.connect(brokerIp, port);
      byte[] payload = message.getBytes();
      aitt.publish(topic, payload);
      aitt.close();
   }

   @Test
   public void testPublishWebRTC() {
      Aitt aitt = null;
      try {
         aitt = new Aitt(appContext, id);
      } catch (InstantiationException e) {
         Log.e(TAG, "Error during testPublishWebRTC", e);
      }
      assertNotNull(aitt);
      aitt.connect(brokerIp, port);
      byte[] payload = message.getBytes();
      aitt.publish(topic, payload, Aitt.Protocol.WEBRTC, Aitt.QoS.AT_MOST_ONCE, false);
      aitt.close();
   }

   @Test(expected = IllegalArgumentException.class)
   public void testPublishInvalidTopic() {
      Aitt aitt = null;
      try {
         aitt = new Aitt(appContext, id);
      } catch (InstantiationException e) {
         Log.e(TAG, "Error during testPublishInvalidTopic", e);
      }
      assertNotNull(aitt);
      aitt.connect(brokerIp, port);
      String _topic = "";
      byte[] payload = message.getBytes();
      aitt.publish(_topic, payload);
      aitt.close();
   }

   @Test
   public void testPublishAnyProtocol() {
      Aitt aitt = null;
      try {
         aitt = new Aitt(appContext, id);
      } catch (InstantiationException e) {
         Log.e(TAG, "Error during testPublishAnyProtocol", e);
      }
      assertNotNull(aitt);
      aitt.connect(brokerIp, port);
      byte[] payload = message.getBytes();
      aitt.publish(topic, payload, Aitt.Protocol.TCP, Aitt.QoS.AT_LEAST_ONCE, false);
      aitt.close();
   }

   @Test
   public void testPublishProtocolSet() {
      Aitt aitt = null;
      try {
         aitt = new Aitt(appContext, id);
      } catch (InstantiationException e) {
         Log.e(TAG, "Error during testPublishProtocolSet", e);
      }
      assertNotNull(aitt);
      aitt.connect(brokerIp, port);
      byte[] payload = message.getBytes();
      EnumSet<Aitt.Protocol> protocols = EnumSet.of(Aitt.Protocol.MQTT, Aitt.Protocol.TCP);
      aitt.publish(topic, payload, protocols, Aitt.QoS.AT_MOST_ONCE, false);
      aitt.close();
   }

   @Test
   public void testSubscribeMqtt() {
      Aitt aitt = null;
      try {
         aitt = new Aitt(appContext, id);
      } catch (InstantiationException e) {
         Log.e(TAG, "Error during testSubscribeMqtt", e);
      }
      assertNotNull(aitt);
      aitt.connect(brokerIp, port);
      aitt.subscribe(topic, new Aitt.SubscribeCallback() {
         @Override
         public void onMessageReceived(AittMessage message) {
            String _topic = message.getTopic();
            byte[] payload = message.getPayload();
            String correlation = message.getCorrelation();
         }
      });
      aitt.close();
   }

   @Test
   public void testSubscribeWebRTC() {
      Aitt aitt = null;
      try {
         aitt = new Aitt(appContext, id);
      } catch (InstantiationException e) {
         Log.e(TAG, "Error during testSubscribeWebRTC", e);
      }
      assertNotNull(aitt);
      aitt.connect(brokerIp, port);

      aitt.subscribe(topic, new Aitt.SubscribeCallback() {
                 @Override
                 public void onMessageReceived(AittMessage message) {
                    String _topic = message.getTopic();
                    byte[] payload = message.getPayload();
                    String correlation = message.getCorrelation();
                 }
              },
              Aitt.Protocol.WEBRTC, Aitt.QoS.AT_MOST_ONCE);

      aitt.close();
   }

   @Test(expected = IllegalArgumentException.class)
   public void testSubscribeInvalidTopic() {
      Aitt aitt = null;
      try {
         aitt = new Aitt(appContext, id);
      } catch (InstantiationException e) {
         Log.e(TAG, "Error during testSubscribeInvalidTopic", e);
      }
      assertNotNull(aitt);
      aitt.connect(brokerIp, port);
      String _topic = "";
      aitt.subscribe(_topic, new Aitt.SubscribeCallback() {
         @Override
         public void onMessageReceived(AittMessage message) {}
      });

      aitt.close();
   }

   @Test(expected = IllegalArgumentException.class)
   public void testSubscribeInvalidCallback() {
      Aitt aitt = null;
      try {
         aitt = new Aitt(appContext, id);
      } catch (InstantiationException e) {
         Log.e(TAG, "Error during testSubscribeInvalidCallback", e);
      }
      assertNotNull(aitt);
      aitt.connect(brokerIp, port);
      String _topic = "";
      aitt.subscribe(_topic, null);
      aitt.close();
   }

   @Test
   public void testSubscribeAnyProtocol() {
      Aitt aitt = null;
      try {
         aitt = new Aitt(appContext, id);
      } catch (InstantiationException e) {
         Log.e(TAG, "Error during testSubscribeAnyProtocol", e);
      }
      assertNotNull(aitt);
      aitt.connect(brokerIp, port);
      aitt.subscribe(topic, new Aitt.SubscribeCallback() {
                 @Override
                 public void onMessageReceived(AittMessage message) {
                    String _topic = message.getTopic();
                    byte[] payload = message.getPayload();
                    String correlation = message.getCorrelation();
                 }
              },
              Aitt.Protocol.UDP, Aitt.QoS.AT_MOST_ONCE);
      aitt.close();
   }

   @Test
   public void testSubscribeProtocolSet() {
      Aitt aitt = null;
      try {
         aitt = new Aitt(appContext, id);
      } catch (InstantiationException e) {
         Log.e(TAG, "Error during testSubscribeProtocolSet", e);
      }
      assertNotNull(aitt);
      aitt.connect(brokerIp, port);
      EnumSet<Aitt.Protocol> protocols = EnumSet.of(Aitt.Protocol.MQTT, Aitt.Protocol.TCP);
      aitt.subscribe(topic, new Aitt.SubscribeCallback() {
                 @Override
                 public void onMessageReceived(AittMessage message) {
                    String _topic = message.getTopic();
                    byte[] payload = message.getPayload();
                    String correlation = message.getCorrelation();
                 }
              },
              protocols, Aitt.QoS.EXACTLY_ONCE);
      aitt.close();
   }

   @Test
   public void testUnsubscribe() {
      Aitt aitt = null;
      try {
         aitt = new Aitt(appContext, id);
      } catch (InstantiationException e) {
         Log.e(TAG, "Error during testUnsubscribe", e);
      }
      assertNotNull(aitt);
      aitt.connect(brokerIp, port);

      aitt.subscribe(topic, new Aitt.SubscribeCallback() {
         @Override
         public void onMessageReceived(AittMessage message) {}
      });
      aitt.unsubscribe(topic);
      aitt.close();
   }

   @Test(expected = IllegalArgumentException.class)
   public void testUnsubscribeInvalidTopic() {
      Aitt aitt = null;
      try {
         aitt = new Aitt(appContext, id);
      } catch (InstantiationException e) {
         Log.e(TAG, "Error during testUnsubscribe", e);
      }
      assertNotNull(aitt);
      aitt.connect(brokerIp, port);
      String _topic = "";
      aitt.unsubscribe(_topic);
      aitt.close();
   }
}
