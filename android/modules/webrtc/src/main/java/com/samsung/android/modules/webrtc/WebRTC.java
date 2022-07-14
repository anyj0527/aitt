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
package com.samsung.android.modules.webrtc;

import android.content.Context;
import android.os.SystemClock;
import android.util.Log;

import org.json.JSONException;
import org.json.JSONObject;
import org.webrtc.CapturerObserver;
import org.webrtc.DataChannel;
import org.webrtc.DefaultVideoDecoderFactory;
import org.webrtc.DefaultVideoEncoderFactory;
import org.webrtc.EglBase;
import org.webrtc.IceCandidate;
import org.webrtc.MediaConstraints;
import org.webrtc.MediaStream;
import org.webrtc.MediaStreamTrack;
import org.webrtc.NV21Buffer;
import org.webrtc.PeerConnection;
import org.webrtc.PeerConnectionFactory;
import org.webrtc.RtpReceiver;
import org.webrtc.SdpObserver;
import org.webrtc.SessionDescription;
import org.webrtc.SurfaceTextureHelper;
import org.webrtc.VideoCapturer;
import org.webrtc.VideoDecoderFactory;
import org.webrtc.VideoEncoderFactory;
import org.webrtc.VideoFrame;
import org.webrtc.VideoSink;
import org.webrtc.VideoSource;
import org.webrtc.VideoTrack;
import static org.webrtc.SessionDescription.Type.ANSWER;
import static org.webrtc.SessionDescription.Type.OFFER;

import java.io.IOException;
import java.io.ObjectInputStream;
import java.io.ObjectOutputStream;
import java.io.Serializable;
import java.net.Socket;
import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.concurrent.TimeUnit;


public class WebRTC {
    private static final String TAG = "WebRTC";
    public static final String VIDEO_TRACK_ID = "ARDAMSv0";
    private static final String CANDIDATE = "candidate";
    private java.net.Socket socket;
    private boolean isInitiator;
    private boolean isChannelReady;
    private boolean isStarted;
    private boolean isReciever;
    private PeerConnection peerConnection;
    private PeerConnectionFactory factory;
    private VideoTrack videoTrackFromSource;
    private ObjectOutputStream outStream;
    private ObjectInputStream inputStream;
    private SDPThread sdpThread;
    private Context appContext;
    private DataChannel localDataChannel;
    private FrameVideoCapturer videoCapturer;
    private ReceiveDataCallback dataCallback;
    private String recieverIP;
    private Integer recieverPort;

    public enum DataType{
        MESSAGE,
        VIDEOFRAME,
    }

    public WebRTC(DataType dataType , Context appContext) {
        this.appContext = appContext;
        this.isReciever = false;
    }

    WebRTC(DataType dataType , Context appContext , Socket socket) {
        Log.d(TAG , "InWebRTC Constructor");
        this.appContext = appContext;
        this.socket = socket;
        this.isReciever = true;
    }

    public void registerDataCallback(ReceiveDataCallback cb){
        this.dataCallback = cb;
    }

    public void disconnect() {
        if (sdpThread != null) {
            sdpThread.stop();
        }

        if (socket != null) {
            new Thread(() -> {
                try {
                    sendMessage(false, "bye");
                    socket.close();
                    if (outStream != null) {
                        outStream.close();
                    }
                    if (inputStream != null) {
                        inputStream.close();
                    }
                } catch (IOException e) {
                    Log.e(TAG, "Error during disconnect", e);
                }
            }).start();
        }
    }

    public void connect() {
        initialize();
    }

    public void connect(String recieverIP , Integer recieverPort){
        this.recieverIP = recieverIP;
        this.recieverPort = recieverPort;
        initialize();
    }

    private void initialize(){
        initializePeerConnectionFactory();
        initializePeerConnections();
        if(!isReciever){
            createVideoTrack();
            addVideoTrack();
        }
        isInitiator = isReciever;

        sdpThread = new SDPThread();
        new Thread(sdpThread).start();
    }

    private void doCall() {
        MediaConstraints sdpMediaConstraints = new MediaConstraints();
        sdpMediaConstraints.mandatory.add(new MediaConstraints.KeyValuePair("OfferToReceiveVideo", "true"));

        peerConnection.createOffer(new SimpleSdpObserver() {
            @Override
            public void onCreateSuccess(SessionDescription sessionDescription) {
                Log.d(TAG, "onCreateSuccess: ");
                peerConnection.setLocalDescription(new SimpleSdpObserver(), sessionDescription);
                JSONObject message = new JSONObject();
                try {
                    message.put("type", "offer");
                    message.put("sdp", sessionDescription.description);
                    sendMessage(true , message);
                } catch (JSONException | IOException e) {
                    Log.e(TAG, "Error during create offer", e);
                }
            }
        }, sdpMediaConstraints);
    }

    private void sendMessage(boolean isJSON , Object message) throws IOException {
        Log.d(TAG , message.toString());
        if(isJSON){
            outStream.writeObject(new Packet((JSONObject) message));
        }
        else{
            outStream.writeObject(new Packet((String) message));
        }
    }

    private class ProxyVideoSink implements VideoSink {

        private ReceiveDataCallback dataCallback;

        ProxyVideoSink(ReceiveDataCallback dataCb){
            this.dataCallback = dataCb;
        }

        @Override
        synchronized public void onFrame(VideoFrame frame) {
            byte[] rawFrame = createNV21Data(frame.getBuffer().toI420());
            dataCallback.pushData(rawFrame);
        }

        public byte[] createNV21Data(VideoFrame.I420Buffer i420Buffer) {
            final int width = i420Buffer.getWidth();
            final int height = i420Buffer.getHeight();
            final int chromaStride = width;
            final int chromaWidth = (width + 1) / 2;
            final int chromaHeight = (height + 1) / 2;
            final int ySize = width * height;
            final ByteBuffer nv21Buffer = ByteBuffer.allocateDirect(ySize + chromaStride * chromaHeight);
            final byte[] nv21Data = nv21Buffer.array();
            for (int y = 0; y < height; ++y) {
                for (int x = 0; x < width; ++x) {
                    final byte yValue = i420Buffer.getDataY().get(y * i420Buffer.getStrideY() + x);
                    nv21Data[y * width + x] = yValue;
                }
            }
            for (int y = 0; y < chromaHeight; ++y) {
                for (int x = 0; x < chromaWidth; ++x) {
                    final byte uValue = i420Buffer.getDataU().get(y * i420Buffer.getStrideU() + x);
                    final byte vValue = i420Buffer.getDataV().get(y * i420Buffer.getStrideV() + x);
                    nv21Data[ySize + y * chromaStride + 2 * x + 0] = vValue;
                    nv21Data[ySize + y * chromaStride + 2 * x + 1] = uValue;
                }
            }
            return nv21Data;
        }
    }


    private void initializePeerConnectionFactory() {
        EglBase mRootEglBase;
        mRootEglBase = EglBase.create();
        VideoEncoderFactory encoderFactory = new DefaultVideoEncoderFactory(mRootEglBase.getEglBaseContext(), true /* enableIntelVp8Encoder */, true);
        VideoDecoderFactory decoderFactory = new DefaultVideoDecoderFactory(mRootEglBase.getEglBaseContext());

        PeerConnectionFactory.initialize(PeerConnectionFactory.InitializationOptions.builder(appContext).setEnableInternalTracer(true).createInitializationOptions());
        PeerConnectionFactory.Builder builder = PeerConnectionFactory.builder().setVideoEncoderFactory(encoderFactory).setVideoDecoderFactory(decoderFactory);
        builder.setOptions(null);
        factory = builder.createPeerConnectionFactory();
    }

    private void createVideoTrack(){
        videoCapturer = new FrameVideoCapturer();
        VideoSource videoSource = factory.createVideoSource(false);
        videoCapturer.initialize(null , null ,videoSource.getCapturerObserver());
        videoTrackFromSource = factory.createVideoTrack(VIDEO_TRACK_ID, videoSource);
        videoTrackFromSource.setEnabled(true);
    }

    private void initializePeerConnections() {
        peerConnection = createPeerConnection(factory);
        localDataChannel = peerConnection.createDataChannel("sendDataChannel", new DataChannel.Init());
    }

    private void addVideoTrack() {
        MediaStream mediaStream = factory.createLocalMediaStream("ARDAMS");
        mediaStream.addTrack(videoTrackFromSource);
        peerConnection.addStream(mediaStream);
    }

    private PeerConnection createPeerConnection(PeerConnectionFactory factory) {
        PeerConnection.RTCConfiguration rtcConfig = new PeerConnection.RTCConfiguration(new ArrayList<>());
        MediaConstraints pcConstraints = new MediaConstraints();

        PeerConnection.Observer pcObserver = new PeerConnection.Observer() {
            @Override
            public void onSignalingChange(PeerConnection.SignalingState signalingState) {
                Log.d(TAG, "onSignalingChange: ");
            }

            @Override
            public void onIceConnectionChange(PeerConnection.IceConnectionState iceConnectionState) {
                Log.d(TAG, "onIceConnectionChange: ");
            }

            @Override
            public void onIceConnectionReceivingChange(boolean b) {
                Log.d(TAG, "onIceConnectionReceivingChange: ");
            }

            @Override
            public void onIceGatheringChange(PeerConnection.IceGatheringState iceGatheringState) {
                Log.d(TAG, "onIceGatheringChange: ");
            }

            @Override
            public void onIceCandidate(IceCandidate iceCandidate) {
                Log.d(TAG, "onIceCandidate: ");
                JSONObject message = new JSONObject();
                try {
                    message.put("type", CANDIDATE);
                    message.put("label", iceCandidate.sdpMLineIndex);
                    message.put("id", iceCandidate.sdpMid);
                    message.put(CANDIDATE, iceCandidate.sdp);
                    Log.d(TAG, "onIceCandidate: sending candidate " + message);
                    sendMessage(true , message);
                } catch (JSONException | IOException e) {
                    Log.e(TAG, "Error during onIceCandidate", e);
                }
            }

            @Override
            public void onIceCandidatesRemoved(IceCandidate[] iceCandidates) {
                Log.d(TAG, "onIceCandidatesRemoved: ");
            }

            @Override
            public void onAddStream(MediaStream mediaStream) {
                Log.d(TAG, "onAddStream: " + mediaStream.videoTracks.size());
                VideoTrack remoteVideoTrack = mediaStream.videoTracks.get(0);
                remoteVideoTrack.setEnabled(true);
            }

            @Override
            public void onRemoveStream(MediaStream mediaStream) {
                Log.d(TAG, "onRemoveStream: ");
            }

            @Override
            public void onDataChannel(DataChannel dataChannel) {
                Log.d(TAG, "onDataChannel: ");
                dataChannel.registerObserver(new DataChannel.Observer() {
                    @Override
                    public void onBufferedAmountChange(long l) {
                        //Keep this callback for future usage
                        Log.d(TAG, "onBufferedAmountChange:");
                    }

                    @Override
                    public void onStateChange() {
                        Log.d(TAG, "onStateChange: remote data channel state: " + dataChannel.state().toString());
                    }

                    @Override
                    public void onMessage(DataChannel.Buffer buffer) {
                        Log.d(TAG, "onMessage: got message");
                        dataCallback.pushData(readIncomingMessage(buffer.data));
                    }
                });
            }

            @Override
            public void onRenegotiationNeeded() {
                Log.d(TAG, "onRenegotiationNeeded: ");
            }

            @Override
            public void onAddTrack(RtpReceiver rtpReceiver, MediaStream[] mediaStreams) {
                MediaStreamTrack track = rtpReceiver.track();
                if (track instanceof VideoTrack && isReciever) {
                    Log.i(TAG, "onAddVideoTrack");
                    VideoTrack remoteVideoTrack = (VideoTrack) track;
                    remoteVideoTrack.setEnabled(true);
                    ProxyVideoSink  videoSink = new ProxyVideoSink(dataCallback);
                    remoteVideoTrack.addSink(videoSink);
                }
            }
        };
        return factory.createPeerConnection(rtcConfig, pcConstraints, pcObserver);
    }

    public void sendVideoData(byte[] frame , int width , int height){
        videoCapturer.send(frame , width , height);
    }

    public void sendMessageData(byte[] message) {
        ByteBuffer data = ByteBuffer.wrap(message);
        localDataChannel.send(new DataChannel.Buffer(data, false));
    }

    public interface ReceiveDataCallback{
        void pushData(byte[] frame);
    }

    private static class Packet implements Serializable {
        boolean isString;
        String obj;
        Packet(String s){
            isString = true;
            obj = s;
        }

        Packet(JSONObject json){
            isString = false;
            obj = json.toString();
        }
    }

    private byte[] readIncomingMessage(ByteBuffer buffer) {
        byte[] bytes;
        if (buffer.hasArray()) {
            bytes = buffer.array();
        } else {
            bytes = new byte[buffer.remaining()];
            buffer.get(bytes);
        }
        return bytes;
    }

    private static class SimpleSdpObserver implements SdpObserver {
        @Override
        public void onCreateSuccess(SessionDescription sessionDescription) {
            //Required for future reference
        }

        @Override
        public void onSetSuccess() {
            Log.d(TAG, "onSetSuccess:");
        }

        @Override
        public void onCreateFailure(String s) {
            Log.d(TAG, "onCreateFailure: Reason = " + s);
        }

        @Override
        public void onSetFailure(String s) {
            Log.d(TAG, "onSetFailure: Reason = " + s);
        }
    }

    private static class FrameVideoCapturer implements VideoCapturer {
        private CapturerObserver capturerObserver;

        void send(byte[] frame, int width, int height) {
            long timestampNS = TimeUnit.MILLISECONDS.toNanos(SystemClock.elapsedRealtime());
            NV21Buffer buffer = new NV21Buffer(frame, width, height, null);
            VideoFrame videoFrame = new VideoFrame(buffer, 0, timestampNS);
            this.capturerObserver.onFrameCaptured(videoFrame);
            videoFrame.release();
        }

        @Override
        public void initialize(SurfaceTextureHelper surfaceTextureHelper, Context context, CapturerObserver capturerObserver) {
            this.capturerObserver = capturerObserver;
        }

        public void startCapture(int width, int height, int framerate) {
            //Required for future reference
        }

        public void stopCapture() throws InterruptedException {
            //Required for future reference
        }

        public void changeCaptureFormat(int width, int height, int framerate) {
            //Required for future reference
        }

        public void dispose() {
            //Required for future reference
        }

        public boolean isScreencast() {
            return false;
        }
    }

    private class SDPThread implements Runnable {
        private volatile boolean isRunning = true;

        @Override
        public void run() {
            isChannelReady = true;

            createSocket();
            invokeSendMessage();

            while(isRunning){
                try {
                    Packet recvPacketNew = (Packet)inputStream.readObject();

                    if(recvPacketNew.isString){
                        String message = recvPacketNew.obj;
                        checkPacketMessage(message);
                    }
                    else{
                        JSONObject message = new JSONObject(recvPacketNew.obj);
                        Log.d(TAG, "connectToSignallingServer: got message " + message);
                        decodeMessage(message);
                    }
                } catch (ClassNotFoundException | JSONException | IOException e) {
                    isRunning = false;
                    Log.e(TAG, "Error during JSON read", e);
                }
            }
        }

        private void decodeMessage(JSONObject message) {
            try {
                if (message.getString("type").equals("offer")) {
                    Log.d(TAG, "connectToSignallingServer: received an offer " + isInitiator + " " + isStarted);
                    invokeMaybeStart();
                    peerConnection.setRemoteDescription(new SimpleSdpObserver(), new SessionDescription(OFFER, message.getString("sdp")));
                    doAnswer();
                } else if (message.getString("type").equals("answer") && isStarted) {
                    peerConnection.setRemoteDescription(new SimpleSdpObserver(), new SessionDescription(ANSWER, message.getString("sdp")));
                } else if (message.getString("type").equals(CANDIDATE) && isStarted) {
                    Log.d(TAG, "connectToSignallingServer: receiving candidates");
                    IceCandidate candidate = new IceCandidate(message.getString("id"), message.getInt("label"), message.getString(CANDIDATE));
                    peerConnection.addIceCandidate(candidate);
                }
            } catch (JSONException e) {
                Log.e(TAG, "Error during message decoding", e);
            }
        }

        private void doAnswer() {
            peerConnection.createAnswer(new SimpleSdpObserver() {
                @Override
                public void onCreateSuccess(SessionDescription sessionDescription) {
                    peerConnection.setLocalDescription(new SimpleSdpObserver(), sessionDescription);
                    JSONObject message = new JSONObject();
                    try {
                        message.put("type", "answer");
                        message.put("sdp", sessionDescription.description);
                        sendMessage(true, message);
                    } catch (JSONException | IOException e) {
                        Log.e(TAG, "Error during sdp answer", e);
                    }
                }
            }, new MediaConstraints());
        }

        private void createSocket(){
            try {
                if(!isReciever){
                    socket = new Socket(recieverIP, recieverPort);
                }
                outStream = new ObjectOutputStream(socket.getOutputStream());
                inputStream = new ObjectInputStream(socket.getInputStream());
            } catch (Exception e) {
                Log.e(TAG, "Error during create socket", e);
            }
        }

        private void invokeSendMessage(){
            try {
                sendMessage(false , "got user media");
            } catch (Exception e) {
                Log.e(TAG, "Error during invoke send message", e);
            }
        }

        private void checkPacketMessage(String message){
            if (message.equals("got user media")) {
                maybeStart();
            }
        }

        private void invokeMaybeStart(){
            if (!isInitiator && !isStarted) {
                maybeStart();
            }
        }

        private void maybeStart() {
            Log.d(TAG, "maybeStart: " + isStarted + " " + isChannelReady);
            if (!isStarted && isChannelReady) {
                isStarted = true;
                if (isInitiator) {
                    doCall();
                }
            }
        }

        public void stop() {
            isRunning = false;
        }
    }
}
