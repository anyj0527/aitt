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

#include <functional>
#include <string>

#include <webrtc.h>

class WebRtcState {
  public:
    enum class Stream {
        IDLE,
        NEGOTIATING,
        PLAYING,
    };

    enum class PeerConnection {
        NEW,
        CONNECTING,
        CONNECTED,
        DISCONNECTED,
        FAILED,
        CLOSED,
    };

    enum class Signaling {
        STABLE,
        CLOSED,
        HAVE_LOCAL_OFFER,
        HAVE_REMOTE_OFFER,
        HAVE_LOCAL_PRANSWER,
        HAVE_REMOTE_PRANSWER,
    };

    enum class IceGathering {
        NEW,
        GATHERING,
        COMPLETE,
    };

    enum class IceConnection {
        NEW,
        CHECKING,
        CONNECTED,
        COMPLETED,
        FAILED,
        DISCONNECTED,
        CLOSED,
    };

  public:
    static Stream ToStreamState(webrtc_state_e state);
    static std::string StreamToStr(WebRtcState::Stream state);
    static Signaling ToSignalingState(webrtc_signaling_state_e state);
    static std::string SignalingToStr(WebRtcState::Signaling state);
    static IceGathering ToIceGatheringState(webrtc_ice_gathering_state_e state);
    static std::string IceGatheringToStr(WebRtcState::IceGathering state);
    static IceConnection ToIceConnectionState(webrtc_ice_connection_state_e state);
    static std::string IceConnectionToStr(WebRtcState::IceConnection state);
};
