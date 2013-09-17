/*
 * CinderCanon.h
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
#include "cinder/ImageIo.h"

#define __MACOS__

#include "EDSDK.h"
#include "EDSDKErrors.h"
#include "EDSDKTypes.h"

#include "CanonDebug.h"

using namespace ci;
using namespace ci::app;
using namespace std;

namespace cinder{
namespace canon{
 
class PhotoHandler
{
public:
    
    virtual void photoTaken(EdsDirectoryItemRef directoryItem, EdsError error) = 0;
    virtual void photoDownloaded(const std::string & downloadPath, EdsError error) = 0;
    virtual std::string photoDownloadDirectory() = 0;
};
    
class CinderCanon {

public:
    
    typedef boost::function<void (EdsDirectoryItemRef directoryItem, EdsError error)> PhotoTakenCallback;
    typedef boost::function<void (const std::string & downloadPath, EdsError error)> ImageDownloadedCallback;

    CinderCanon();
    
    void setup( int cameraIndex = 0 );
    void shutdown();
    
    void startLiveView();
    void endLiveView();
    bool isLiveViewing()const { return bIsLiveView; };
    bool getFrameNew() { return bFrameNew; }
    void setFrameNew(bool b) { bFrameNew = b; }
    
    void takePicture(PhotoHandler * photoHandler);
    void downloadImage(EdsDirectoryItemRef dirItem, PhotoHandler * photoHandler);
    
    void downloadData(){ downloadEvfData(mCamera); };
    Surface8u getLiveSurface() const{ return mLivePixels; }

    bool isCameraConnected(){ return bCameraIsConnected; };
    int getNumConnectedCameras();
    
  protected:

    Surface8u   mLivePixels;
    
    void    getDeviceInfo( EdsCameraRef cam );
    bool    sendCommand( EdsCameraRef inCameraRef, EdsUInt32 inCommand, EdsUInt32 inParam );
    
    EdsError downloadEvfData( EdsCameraRef camera );
    
    EdsCameraRef        mCamera;
    EdsCameraListRef    mCamera_list;

    bool                bCameraIsConnected;
    bool                bIsLiveView;
    bool                bFrameNew;
    
    static EdsError EDSCALLBACK handleObjectEvent(
                                                   EdsUInt32 inEvent
                                                  ,EdsBaseRef inRef
                                                  ,EdsVoid* inContext
                                                  );
    
    static EdsError EDSCALLBACK handlePropertyEvent(
                                                     EdsUInt32 inEvent
                                                    ,EdsUInt32 inPropertyID
                                                    ,EdsUInt32 inParam
                                                    ,EdsVoid* inContext
                                                    );
    static EdsError EDSCALLBACK handleStateEvent(
                                                    EdsUInt32 inEvent
                                                   ,EdsUInt32 inParam
                                                   ,EdsVoid* inContext
                                                              );

};

}
}