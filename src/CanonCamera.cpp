/*
 * CanonCamera.cpp
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

#include "CanonCamera.h"

void CanonCamera::setup() {
    reset();
    mCanon.setup();
    mRecordingFPS = 12;
}

// ------------------------------------------------------------------------------
#pragma mark
#pragma mark Starting and Stopping
void CanonCamera::reset() {
    mCapturedFrames.clear();
    stopRecording();
}

void CanonCamera::startLiveView() {
    mCanon.startLiveView();
}

void CanonCamera::endLiveView() {
    mCanon.endLiveView();
}
void CanonCamera::toggleLiveView() {
    if(isLiveViewing()) mCanon.endLiveView();
    else mCanon.startLiveView();
}

void CanonCamera::stop(){
    mCanon.shutdown();
}

void CanonCamera::shutdown(){
    mCanon.shutdown();
}

bool CanonCamera::checkNewFrame(){
    if(mCanon.getFrameNew() == true) {
        mCanon.setFrameNew(false);
        return true;
    } else {
        return false;
    }
}

Surface8u CanonCamera::getLiveSurface(){
    return mCanon.getLiveSurface();
}

// ------------------------------------------------------------------------------
#pragma mark
#pragma mark Photo and Recording
void CanonCamera::takePicture(cinder::canon::PhotoHandler * photoHandler){
    mCanon.takePicture(photoHandler);
}

void CanonCamera::downloadImage(EdsDirectoryItemRef dirItem, cinder::canon::PhotoHandler * photoHandler) {
    mCanon.downloadImage(dirItem, photoHandler);
}

void CanonCamera::startRecording() {
    if(!bRecording) {
        bRecording = true;
        mStartTime = getElapsedSeconds();
    }
}

void CanonCamera::stopRecording() {
    bRecording = false;
}

void CanonCamera::toggleRecording() {
    if(bRecording) stopRecording();
    else startRecording();
}

void CanonCamera::pushFrame(const Surface &frame) {
    mCapturedFrames.push_back(frame);
}

void CanonCamera::saveFrame() {    
    Surface sf = mCanon.getLiveSurface();
    float h = sf.getWidth() * 9.0f / 16.0f;
    
    Surface frame(sf.getWidth(),h,false);
    frame.copyFrom( sf, Area(0,0,sf.getWidth(), h) );
    pushFrame( frame );
}

// ------------------------------------------------------------------------------
#pragma mark
#pragma mark Getters
bool CanonCamera::isLiveViewing() {
    return mCanon.isLiveViewing();
}

int CanonCamera::getWidth() {
    if( mCanon.getLiveSurface() ){
        return mCanon.getLiveSurface().getWidth();
    }
    return 0;
}

int CanonCamera::getHeight() {
    if( mCanon.getLiveSurface() ){
        return mCanon.getLiveSurface().getHeight();
    }
    return 0;
}

// ------------------------------------------------------------------------------
#pragma mark
#pragma mark Update and Draw
void CanonCamera::update() {
    if(bRecording) {
        if( (getElapsedSeconds()-mStartTime) >= 1.0f/ mRecordingFPS ){
            saveFrame();
            mStartTime = getElapsedSeconds();
        }
    }
    if(isLiveViewing()){
        mCanon.downloadData();
    }
}

void CanonCamera::draw(Rectf drawingRect) {
    gl::color( Color::white() );
    Surface sf = mCanon.getLiveSurface();
    if( sf ) {
        Rectf sfRect = sf.getBounds();
        if(drawingRect.getWidth() == 0) gl::draw( gl::Texture( sf ));
        else gl::draw( gl::Texture( sf ), Area(sfRect.getCenteredFit( drawingRect, true )) );
    }
}

