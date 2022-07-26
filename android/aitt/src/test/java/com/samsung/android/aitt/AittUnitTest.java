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
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyLong;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.mock;

import android.content.Context;

import org.junit.Test;
import org.junit.runner.RunWith;
import org.mockito.Mock;
import org.powermock.api.mockito.PowerMockito;
import org.powermock.core.classloader.annotations.PrepareForTest;
import org.powermock.modules.junit4.PowerMockRunner;

import java.util.EnumSet;

@RunWith(PowerMockRunner.class)
@PrepareForTest(Aitt.class)
public class AittUnitTest {
   @Mock
   private Context appContext = mock(Context.class);

   private final String brokerIp = "192.168.0.1";
   private final int port = 1803;
   private final String topic = "aitt/test";
   private final String message = "test message";

   private static void initialize(Aitt aitt) {
      try {
         PowerMockito.when(aitt, "initJNI", anyString(), anyString(), anyBoolean()).thenReturn(1L);
         PowerMockito.doNothing().when(aitt, "connectJNI", anyLong(), anyString(), anyInt());
         PowerMockito.doNothing().when(aitt, "disconnectJNI", anyLong());
         PowerMockito.doNothing().when(aitt, "setConnectionCallbackJNI", anyLong());
         PowerMockito.doNothing().when(aitt, "publishJNI", anyLong(), anyString(), any(byte[].class), anyLong(), anyInt(), anyInt(), anyBoolean());
         PowerMockito.when(aitt, "subscribeJNI", anyLong(), anyString(), anyInt(), anyInt()).thenReturn(1L);
         PowerMockito.doNothing().when(aitt, "unsubscribeJNI", anyLong(), anyLong());
      } catch (Exception e) {
         fail("Failed to mock Aitt " + e);
      }
   }

   @Test
   public void testInitialize_P01() {
      try {
         Aitt aitt = PowerMockito.mock(Aitt.class);
         initialize(aitt);

         assertNotNull("Aitt Instance not null", aitt);
         aitt.close();
      } catch (Exception e) {
         fail("Failed testInitialize " + e);
      }
   }

   @Test(expected = IllegalArgumentException.class)
   public void testInitializeInvalidId_N01() {
      String _id = "";
      try {
         Aitt aitt = new Aitt(appContext, _id);
         aitt.close();
      } catch (InstantiationException e) {
         fail("Error during testInitializeInvalidId" + e);
      }
   }

   @Test(expected = IllegalArgumentException.class)
   public void testInitializeInvalidContext_N02() {
      String _id = "";
      try {
         Aitt aitt = new Aitt(null, _id);
         aitt.close();
      } catch (InstantiationException e) {
         fail( "Error during testInitializeInvalidContext" + e);
      }
   }

   @Test
   public void testConnect_P02() {
      try {
         Aitt aitt = PowerMockito.mock(Aitt.class);
         initialize(aitt);

         assertNotNull("Aitt Instance not null", aitt);
         aitt.connect(brokerIp, port);

         aitt.close();
      } catch(Exception e) {
         fail("Failed testConnect " + e);
      }
   }

   @Test
   public void testConnectWithoutIP_P03() {
      try {
         Aitt aitt = PowerMockito.mock(Aitt.class);
         initialize(aitt);

         assertNotNull("Aitt Instance not null", aitt);
         aitt.connect(null);

         aitt.close();
      } catch (Exception e) {
         fail("Failed testConnectWithoutIP " + e);
      }
   }

   @Test
   public void testDisconnect_P04() {
      try {
         Aitt aitt = PowerMockito.mock(Aitt.class);
         initialize(aitt);

         assertNotNull("Aitt Instance not null", aitt);
         aitt.connect(brokerIp, port);

         aitt.disconnect();
      } catch (Exception e) {
         fail("Failed testDisconnect " + e);
      }
   }

   @Test
   public void testPublishMqtt_P05() {
      try {
         Aitt aitt = PowerMockito.mock(Aitt.class);
         initialize(aitt);

         assertNotNull("Aitt Instance not null", aitt);
         aitt.connect(brokerIp, port);

         byte[] payload = message.getBytes();
         aitt.publish(topic, payload);

         aitt.disconnect();
      } catch (Exception e) {
         fail("Failed testPublishMqtt " + e);
      }
   }

   @Test
   public void testPublishWebRTC_P06() {
      try {
         Aitt aitt = PowerMockito.mock(Aitt.class);
         initialize(aitt);

         assertNotNull("Aitt Instance not null", aitt);
         aitt.connect(brokerIp, port);

         byte[] payload = message.getBytes();
         aitt.publish(topic, payload, Aitt.Protocol.WEBRTC, Aitt.QoS.AT_MOST_ONCE, false);

         aitt.disconnect();
      } catch (Exception e) {
         fail("Failed testPublishWebRTC " + e);
      }
   }

   @Test
   public void testPublishInvalidTopic_N03() throws IllegalArgumentException {
      try {
         Aitt aitt = PowerMockito.mock(Aitt.class);
         initialize(aitt);

         assertNotNull("Aitt Instance not null", aitt);
         aitt.connect(brokerIp, port);

         String _topic = "";
         byte[] payload = message.getBytes();
         aitt.publish(_topic, payload);
         aitt.disconnect();
      } catch (Exception e) {
         fail("Failed testPublishInvalidTopic " + e);
      }
   }

   @Test
   public void testPublishAnyProtocol_P07() {
      try {
         Aitt aitt = PowerMockito.mock(Aitt.class);
         initialize(aitt);

         assertNotNull("Aitt Instance not null", aitt);
         aitt.connect(brokerIp, port);

         byte[] payload = message.getBytes();
         aitt.publish(topic, payload, Aitt.Protocol.TCP, Aitt.QoS.AT_LEAST_ONCE, false);

         aitt.disconnect();
      } catch (Exception e) {
         fail("Failed testPublishAnyProtocol " + e);
      }
   }

   @Test
   public void testPublishProtocolSet_P08() {
      try {
         Aitt aitt = PowerMockito.mock(Aitt.class);
         initialize(aitt);

         assertNotNull("Aitt Instance not null", aitt);
         aitt.connect(brokerIp, port);

         byte[] payload = message.getBytes();
         EnumSet<Aitt.Protocol> protocols = EnumSet.of(Aitt.Protocol.MQTT, Aitt.Protocol.TCP);
         aitt.publish(topic, payload, protocols, Aitt.QoS.AT_MOST_ONCE, false);

         aitt.disconnect();
      } catch (Exception e) {
         fail("Failed testPublishProtocolSet " + e);
      }
   }

   @Test
   public void testSubscribeMqtt_P09() {
      try {
         Aitt aitt = PowerMockito.mock(Aitt.class);
         initialize(aitt);

         assertNotNull("Aitt Instance not null", aitt);
         aitt.connect(brokerIp, port);

         aitt.subscribe(topic, new Aitt.SubscribeCallback() {
            @Override
            public void onMessageReceived(AittMessage message) {
               String _topic = message.getTopic();
               byte[] payload = message.getPayload();
            }
         });

         aitt.disconnect();
      } catch (Exception e) {
         fail("Failed testSubscribeMqtt " + e);
      }
   }

   @Test
   public void testSubscribeWebRTC_P10() {
      try {
         Aitt aitt = PowerMockito.mock(Aitt.class);
         initialize(aitt);

         assertNotNull("Aitt Instance not null", aitt);
         aitt.connect(brokerIp, port);

         aitt.subscribe(topic, new Aitt.SubscribeCallback() {
                    @Override
                    public void onMessageReceived(AittMessage message) {
                       String _topic = message.getTopic();
                       byte[] payload = message.getPayload();
                    }
                 },
                 Aitt.Protocol.WEBRTC, Aitt.QoS.AT_MOST_ONCE);

         aitt.disconnect();
      } catch (Exception e) {
         fail("Failed testSubscribeWebRTC " + e);
      }
   }

   @Test
   public void testSubscribeInvalidTopic_N04() throws IllegalArgumentException {
      try {
         Aitt aitt = PowerMockito.mock(Aitt.class);
         initialize(aitt);

         assertNotNull("Aitt Instance not null", aitt);
         aitt.connect(brokerIp, port);

         String _topic = "";
         aitt.subscribe(_topic, new Aitt.SubscribeCallback() {
            @Override
            public void onMessageReceived(AittMessage message) {
            }
         });

         aitt.disconnect();
      } catch (Exception e) {
         fail("Failed testSubscribeInvalidTopic " + e);
      }
   }

   @Test
   public void testSubscribeInvalidCallback_N05() throws IllegalArgumentException {
      try {
         Aitt aitt = PowerMockito.mock(Aitt.class);
         initialize(aitt);

         assertNotNull("Aitt Instance not null", aitt);
         aitt.connect(brokerIp, port);

         String _topic = "";
         aitt.subscribe(_topic, null);

         aitt.disconnect();
      } catch (Exception e) {
         fail("Failed testSubscribeInvalidCallback " + e);
      }
   }

   @Test
   public void testSubscribeAnyProtocol_P11() {
      try {
         Aitt aitt = PowerMockito.mock(Aitt.class);
         initialize(aitt);

         assertNotNull("Aitt Instance not null", aitt);
         aitt.connect(brokerIp, port);

         aitt.subscribe(topic, new Aitt.SubscribeCallback() {
                    @Override
                    public void onMessageReceived(AittMessage message) {
                       String _topic = message.getTopic();
                       byte[] payload = message.getPayload();
                    }
                 },
                 Aitt.Protocol.UDP, Aitt.QoS.AT_MOST_ONCE);

         aitt.disconnect();
      } catch (Exception e) {
         fail("Failed testSubscribeAnyProtocol " + e);
      }
   }

   @Test
   public void testSubscribeProtocolSet_P12() {
      try {
         Aitt aitt = PowerMockito.mock(Aitt.class);
         initialize(aitt);

         assertNotNull("Aitt Instance not null", aitt);
         aitt.connect(brokerIp, port);

         EnumSet<Aitt.Protocol> protocols = EnumSet.of(Aitt.Protocol.MQTT, Aitt.Protocol.TCP);
         aitt.subscribe(topic, new Aitt.SubscribeCallback() {
                    @Override
                    public void onMessageReceived(AittMessage message) {
                       String _topic = message.getTopic();
                       byte[] payload = message.getPayload();
                    }
                 },
                 protocols, Aitt.QoS.EXACTLY_ONCE);

         aitt.disconnect();
      } catch (Exception e) {
         fail("Failed testSubscribeProtocolSet " + e);
      }
   }

   @Test
   public void testUnsubscribe_P13() {
      try {
         Aitt aitt = PowerMockito.mock(Aitt.class);
         initialize(aitt);

         assertNotNull("Aitt Instance not null", aitt);
         aitt.connect(brokerIp, port);

         aitt.subscribe(topic, new Aitt.SubscribeCallback() {
            @Override
            public void onMessageReceived(AittMessage message) {
            }
         });

         aitt.unsubscribe(topic);
         aitt.disconnect();
      } catch (Exception e) {
         fail("Failed testUnsubscribe " + e);
      }
   }

   @Test
   public void testUnsubscribeInvalidTopic_N06() throws IllegalArgumentException {
      try {
         Aitt aitt = PowerMockito.mock(Aitt.class);
         initialize(aitt);

         assertNotNull("Aitt Instance not null", aitt);
         aitt.connect(brokerIp, port);

         String _topic = "";
         aitt.unsubscribe(_topic);
         aitt.disconnect();
      } catch (Exception e) {
         fail("Failed testUnsubscribeInvalidTopic " + e);
      }
   }

   @Test
   public void testSetConnectionCallback_P14() {
      try {
         Aitt aitt = PowerMockito.mock(Aitt.class);
         initialize(aitt);

         assertNotNull("Aitt Instance not null", aitt);
         aitt.setConnectionCallback(new Aitt.ConnectionCallback() {
            @Override
            public void onConnected() {}

            @Override
            public void onDisconnected() {}
         });
         aitt.connect(brokerIp, port);

         aitt.disconnect();
      } catch (Exception e) {
         fail("Failed testSetConnectionCallback " + e);
      }
   }

   @Test
   public void testSetConnectionCallbackInvalidCallback_N07() throws IllegalArgumentException {
      try {
         Aitt aitt = PowerMockito.mock(Aitt.class);
         initialize(aitt);

         assertNotNull("Aitt Instance not null", aitt);
         aitt.setConnectionCallback(null);
         aitt.connect(brokerIp, port);

         aitt.disconnect();
      } catch (Exception e) {
         fail("Failed testSetConnectionCallbackInvalidCallback " + e);
      }
   }
}
