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
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

// TODO: webrtc.h is very heavy header file.
// I think we need to decide whether to include this or not
#include <webrtc.h>

#include "CameraHandler.h"
#include "WebRtcEventHandler.h"

class WebRtcStream {
  public:
    WebRtcStream();
    ~WebRtcStream();
    bool Create(bool is_source, bool need_display);
    void Destroy(void);
    bool Start(void);
    bool Stop(void);
    bool AttachCameraSource(void);
    bool AttachCameraPreviewSource(void);
    static void OnMediaPacketPreview(media_packet_h media_packet, void *user_data);
    bool DetachCameraSource(void);
    void SetDisplayObject(unsigned int id, void *object);
    void AttachSignals(bool is_source, bool need_display);
    // Cautions : Event handler is not a pointer. So, change event_handle after Set Event handler
    // doesn't affect event handler which is included int WebRtcStream
    void SetEventHandler(WebRtcEventHandler event_handler) { event_handler_ = event_handler; };
    WebRtcEventHandler &GetEventHandler(void) { return event_handler_; };

    bool CreateOfferAsync(std::function<void(std::string)> on_created_cb);
    void CallOnOfferCreatedCb(std::string offer)
    {
        if (on_offer_created_cb_)
            on_offer_created_cb_(offer);
    }
    bool CreateAnswerAsync(std::function<void(std::string)> on_created_cb);
    void CallOnAnswerCreatedCb(std::string answer)
    {
        if (on_answer_created_cb_)
            on_answer_created_cb_(answer);
    }
    void SetPreparedLocalDescription(const std::string &description)
    {
        local_description_ = description;
    };
    std::string GetPreparedLocalDescription(void) const { return local_description_; };

    bool SetLocalDescription(const std::string &description);
    bool SetRemoteDescription(const std::string &description);

    bool AddIceCandidateFromMessage(const std::string &ice_message);
    const std::vector<std::string> &GetIceCandidates() const { return ice_candidates_; };

    std::string GetRemoteDescription(void) const { return remote_description_; };

  private:
    static void OnOfferCreated(webrtc_h webrtc, const char *description, void *user_data);
    static void OnAnswerCreated(webrtc_h webrtc, const char *description, void *user_data);
    static void OnError(webrtc_h webrtc, webrtc_error_e error, webrtc_state_e state,
          void *user_data);
    static void OnStateChanged(webrtc_h webrtc, webrtc_state_e previous, webrtc_state_e current,
          void *user_data);
    static void OnSignalingStateChanged(webrtc_h webrtc, webrtc_signaling_state_e state,
          void *user_data);
    static void OnIceConnectionStateChanged(webrtc_h webrtc, webrtc_ice_connection_state_e state,
          void *user_data);
    static void OnIceCandiate(webrtc_h webrtc, const char *candidate, void *user_data);
    static void OnEncodedFrame(webrtc_h webrtc, webrtc_media_type_e type, unsigned int track_id,
          media_packet_h packet, void *user_data);
    static void OnTrackAdded(webrtc_h webrtc, webrtc_media_type_e type, unsigned int id,
          void *user_data);
    static void OnMediaPacketBufferStateChanged(unsigned int source_id,
          webrtc_media_packet_source_buffer_state_e state, void *user_data);

  private:
    webrtc_h webrtc_handle_;
    std::unique_ptr<CameraHandler> camera_handler_;
    // DO we need to make is_source_overflow_ as atomic?
    bool is_source_overflow_;
    unsigned int source_id_;
    std::string local_description_;
    std::string remote_description_;
    std::vector<std::string> ice_candidates_;
    std::function<void(std::string)> on_offer_created_cb_;
    std::function<void(std::string)> on_answer_created_cb_;
    WebRtcEventHandler event_handler_;
};
