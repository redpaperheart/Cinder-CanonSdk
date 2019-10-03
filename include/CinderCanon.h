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

#include "cinder/app/App.h"
#include "cinder/ImageIo.h"
#include "cinder/Signals.h"

//define macos on apple, used in EDSDK headers
#ifdef  __APPLE__
	#define __MACOS__
#endif

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
   
using CinderCanonRef = std::shared_ptr<class CinderCanon>;

class CinderCanon : public canon::PhotoHandler {

public:
	//shared pointer creation
	
	static CinderCanonRef create() {
		return std::make_shared<CinderCanon>();
	}

	typedef std::function<void (EdsDirectoryItemRef directoryItem, EdsError error)> PhotoTakenCallback;
	typedef std::function<void (const std::string & downloadPath, EdsError error)> ImageDownloadedCallback;

	CinderCanon();
	~CinderCanon() {
		console() << "Cinder-Canon :: good bye" << endl;
		//EdsCloseSession(mCamera);
		shutdown();
	}
	
	void setup( int cameraIndex = 0 );
	void shutdown();
	bool hearbeat() {
		EdsError err = EDS_ERR_OK;
		char buf[800];

		err = EdsGetPropertyData(mCamera, kEdsPropID_BodyIDEx, 0, 800, &buf);

		if (err != EDS_ERR_OK) {
			//console() << "Cinder-Canon :: Couldn't retrieve Body ID" << endl;
			return false;
		}
		//console() << "heartbeat" << endl;
		return true;
	}
	
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

	void onCameraDisconnected();

	int deviceIndex = -1;
	std::string deviceBodyId = "none";
	std::string downloadDirectory = "";

	bool autoDownload = true;
	bool deleteAfterDownload = true;
	//create getters?

	//photo handler delegate methods
	void photoTaken(EdsDirectoryItemRef directoryItem, EdsError error);
	void photoDownloaded(const std::string& downloadPath, EdsError error);
	std::string photoDownloadDirectory();

	//signals
	ci::signals::Signal<void(std::string)> signal_photoTaken; //deviceBodyId
	ci::signals::Signal<void(std::string, std::string)> signal_photoDownloaded; //deviceBodyId, photoPath

  protected:

	Surface8u   mLivePixels;
	
	void    getDeviceInfo( EdsCameraRef cam );
	void    storeDeviceSerial(EdsCameraRef cam);
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
