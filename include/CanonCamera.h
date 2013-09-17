/*
 * CanonCamera.h
 *
 * Created by Red Paper Heart
 *
 * Copyright (c) 2012, Red Paper Heart, All rights reserved.
 * This code is intended for use with the Cinder C++ library: http://libcinder.org
 *
 * To contact Red Paper Heart, email hello@redpaperheart.com or tweet @redpaperhearts
 *
 * Redistribution and use in source and binary forms, with or
 * without modification, are permitted provided that the following
 * conditions are met:
 *
 * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in
 * the documentation and/or other materials provided with the
 * distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#pragma once

#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"

#include "CinderCanon.h"

using namespace ci;
using namespace std;

class CanonCamera {
public:
    void setup();
    void update();
    void draw(Rectf drawingRect = Rectf(0,0,0,0));
    
    void reset();
    void stop();
    void shutdown();
    
    void startLiveView();
    void endLiveView();
    void toggleLiveView();
    
    // recording - not tested yet
    void stopRecording();
    void startRecording();
    void toggleRecording();
    
    void takePicture(canon::PhotoHandler * photoHandler);
    void downloadImage(EdsDirectoryItemRef dirItem, canon::PhotoHandler * photoHandler);

    bool isBusy() { return false; }
    bool isLiveViewing();
    bool isRecording() { return bRecording; }
    
    bool checkNewFrame();
    int getWidth();
    int getHeight();
    int getTotalCapturedFrames() { return mCapturedFrames.size(); }
    Surface8u getLiveSurface() ;
    const vector<Surface>& getCapturedFrames() { return mCapturedFrames; }
    
    // for advanced use
    canon::CinderCanon getCannon(){ return mCanon; };

    
protected:
    bool    bRecording;
    bool    bFrameNew;
    int     mRecordingFPS;
    float   mStartTime;
    canon::CinderCanon mCanon;
    vector<Surface> mCapturedFrames;
    
    void saveFrame();
    void pushFrame(const Surface &frame);

};
