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
#include "cinder/Log.h"
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
//using namespace ci::app;
//using namespace std;

namespace cinder {
	namespace canon {
 
		//class PhotoHandler {
		//  public:
		//	virtual void photoTaken(EdsDirectoryItemRef directoryItem, EdsError error) = 0;
		//	virtual void photoDownloaded(const std::string & downloadPath, EdsError error) = 0;
		//	virtual std::string photoDownloadDirectory() = 0;
		//};
   
		using CanonCameraRef = std::shared_ptr<class CanonCamera>;

		class CanonCamera { //: public canon::PhotoHandler {
		  public:
			static CanonCameraRef create() {
				return std::make_shared<canon::CanonCamera>();
			}

			typedef std::function<void (EdsDirectoryItemRef directoryItem, EdsError error)> PhotoTakenCallback;
			typedef std::function<void (const std::string & downloadPath, EdsError error)> ImageDownloadedCallback;

			CanonCamera();
			~CanonCamera() {
				CI_LOG_D(deviceBodyId << " :: good bye.");
				shutdown();
			}
	
			void setup( int cameraIndex = 0 );
			void setup(EdsCameraRef cameraRef);
			void shutdown( bool terminateSdk = false );
			bool hearbeat() {
				EdsError err = EDS_ERR_OK;
				char buf[800];
				err = EdsGetPropertyData(mCamera, kEdsPropID_BodyIDEx, 0, 800, &buf);
				if (err != EDS_ERR_OK) {
					if (err == EDS_ERR_INVALID_HANDLE) {
						onCameraDisconnected();
						return false;
					}
					return false;
				}
				return true;
			}
	
			void startLiveView();
			void endLiveView();
			bool isLiveViewing()const { return bIsLiveView; };
			bool getFrameNew() { return bFrameNew; }
			void setFrameNew(bool b) { bFrameNew = b; }

			EdsError lockUI();
			EdsError unlockUI();
	
			bool takePicture(); // PhotoHandler* photoHandler);
			void downloadImage(EdsDirectoryItemRef dirItem); // , PhotoHandler* photoHandler);
	
			void downloadData(){ downloadEvfData(mCamera); };
			Surface8u getLiveSurface() const{ return mLivePixels; }

			bool isCameraConnected(){ return bCameraIsConnected; };
			int getNumConnectedCameras();

			EdsError getPropertyFromCamera(EdsPropertyID propertyID, EdsInt32  inParam = 0);
			EdsError getPropertyDescFromCamera(EdsPropertyID propertyID);
			bool setProperty(EdsPropertyID propertyID, EdsUInt32 data)
			{
				CI_LOG_D(deviceBodyId << " :: " << CanonPropertyToString(propertyID));
				EdsError err = EDS_ERR_OK;
				err = EdsSetPropertyData(mCamera, propertyID, 0, sizeof(data), (EdsVoid*)&data);

				//Notification of error
				if (err != EDS_ERR_OK) {
					CI_LOG_E("Setting " << CanonPropertyToString(propertyID) << " failed.");
					return false;
				}
				return true;
			}

			

			static bool isSDKLoaded;

			//int deviceIndex = -1;
			std::string deviceBodyId = "none";
			std::string downloadDirectory = "";

			bool autoDownload = true;
			bool deleteAfterDownload = true;
			std::string getPhotoDownloadDirectory();

			//photo handler delegate methods
			void photoTaken(EdsDirectoryItemRef directoryItem, EdsError error);
			void photoDownloaded(const std::string& downloadPath, EdsError error);

			//signals
			ci::signals::Signal<void(std::string)> signal_photoTaken; //deviceBodyId
			ci::signals::Signal<void(std::string, std::string)> signal_photoDownloaded; //deviceBodyId, photoPath
			ci::signals::Signal<void(std::string)> signal_disconnected; //deviceBodyId

		  protected:
			void onCameraDisconnected();
			void getDeviceInfo( EdsCameraRef cam );
			bool sendCommand( EdsCameraRef inCameraRef, EdsUInt32 inCommand, EdsUInt32 inParam );
	
			EdsError downloadEvfData( EdsCameraRef camera );

			Surface8u mLivePixels;

			bool mUILocked = false;
			
			// Model name
			std::string _modelName;

			// Taking a picture parameter
			EdsUInt32 _AEMode;
			EdsUInt32 _Av;
			EdsUInt32 _Tv;
			EdsUInt32 _Iso;
			EdsUInt32 _MeteringMode;
			EdsUInt32 _ExposureCompensation;
			EdsUInt32 _ImageQuality;
			EdsUInt32 _AvailableShot;
			EdsUInt32 _evfMode;
			EdsUInt32 _evfOutputDevice;
			EdsUInt32 _evfDepthOfFieldPreview;
			EdsUInt32 _evfZoom;
			EdsPoint  _evfZoomPosition;
			EdsRect	  _evfZoomRect;
			EdsUInt32 _evfAFMode;
			EdsUInt32 _AFMode;

			EdsFocusInfo _focusInfo;

			// List of value in which taking a picture parameter can be set
			EdsPropertyDesc _AEModeDesc;
			EdsPropertyDesc _AvDesc;
			EdsPropertyDesc _TvDesc;
			EdsPropertyDesc _IsoDesc;
			EdsPropertyDesc _MeteringModeDesc;
			EdsPropertyDesc _ExposureCompensationDesc;
			EdsPropertyDesc _ImageQualityDesc;
			EdsPropertyDesc _evfAFModeDesc;

			EdsCameraRef        mCamera;
			EdsCameraListRef    mCamera_list;

			bool                bCameraIsConnected;
			bool                bIsLiveView;
			bool                bFrameNew;
	
			static EdsError EDSCALLBACK handleObjectEvent(EdsUInt32 inEvent, EdsBaseRef inRef ,EdsVoid* inContext);
			static EdsError EDSCALLBACK handlePropertyEvent(EdsUInt32 inEvent, EdsUInt32 inPropertyID ,EdsUInt32 inParam ,EdsVoid* inContext);
			static EdsError EDSCALLBACK handleStateEvent(EdsUInt32 inEvent, EdsUInt32 inParam, EdsVoid* inContext);


			//Property
		  public:

			EdsCameraRef getCameraRef() { return mCamera; }

			// Taking a picture parameter
			void setAEMode(EdsUInt32 value) { _AEMode = value; }
			void setTv(EdsUInt32 value) { 
				_Tv = value; 
				CI_LOG_D(deviceBodyId << " :: _Tv: " << CanonTvPropertyDataToString(value));
			}
			void setAv(EdsUInt32 value) { 
				_Av = value; 
				CI_LOG_D(deviceBodyId << " :: _Av: " << CanonAvPropertyDataToString(value));
			}
			void setIso(EdsUInt32 value) { 
				_Iso = value; 
				CI_LOG_D(deviceBodyId << " :: _Iso: " << CanonIsoPropertyDataToString(value)); 
			}
			void setExposureCompensation(EdsUInt32 value) { 
				_ExposureCompensation = value; 
				CI_LOG_D(deviceBodyId << " :: _ExposureCompensation: " << value); 
			}
			void setAFMode(EdsUInt32 value) { _AFMode = value; CI_LOG_D(deviceBodyId << " :: _AFMode: " << value << " :: " << CanonAFModeDataToString(value)); }
			void setMeteringMode(EdsUInt32 value) { _MeteringMode = value; CI_LOG_D(deviceBodyId << " :: _MeteringMode: " << value); }
			void setImageQuality(EdsUInt32 value) { _ImageQuality = value; CI_LOG_D(deviceBodyId << " :: _ImageQuality: " << value); }
			void setEvfMode(EdsUInt32 value) { _evfMode = value; CI_LOG_D(deviceBodyId << " :: _evfMode: " << value); }
			void setEvfOutputDevice(EdsUInt32 value) { _evfOutputDevice = value; CI_LOG_D(deviceBodyId << " :: _evfOutputDevice: " << value); }
			void setEvfDepthOfFieldPreview(EdsUInt32 value) { _evfDepthOfFieldPreview = value; CI_LOG_D(deviceBodyId << " :: _evfDepthOfFieldPreview: " << value); }
			void setEvfZoom(EdsUInt32 value) { _evfZoom = value; CI_LOG_D(deviceBodyId << " :: _evfZoom: " << value); }
			void setEvfZoomPosition(EdsPoint value) { _evfZoomPosition = value; }
			void setEvfZoomRect(EdsRect value) { _evfZoomRect = value; }
			void setModelName(EdsChar* modelName) { _modelName = std::string(modelName); }
			void setEvfAFMode(EdsUInt32 value) { _evfAFMode = value; CI_LOG_D(deviceBodyId << " :: _evfAFMode: " << value); }
			void setFocusInfo(EdsFocusInfo value) { _focusInfo = value; }

			// Taking a picture parameter
			EdsUInt32 getAEMode() const { return _AEMode; }
			EdsUInt32 getTv() const { return _Tv; }
			EdsUInt32 getAv() const { return _Av; }
			EdsUInt32 getIso() const { return _Iso; }
			EdsUInt32 getMeteringMode() const { return _MeteringMode; }
			EdsUInt32 getExposureCompensation() const { return _ExposureCompensation; }
			EdsUInt32 getImageQuality() const { return _ImageQuality; }
			EdsUInt32 getEvfMode() const { return _evfMode; }
			EdsUInt32 getEvfOutputDevice() const { return _evfOutputDevice; }
			EdsUInt32 getEvfDepthOfFieldPreview() const { return _evfDepthOfFieldPreview; }
			EdsUInt32 getEvfZoom() const { return _evfZoom; }
			EdsPoint  getEvfZoomPosition() const { return _evfZoomPosition; }
			EdsRect	  getEvfZoomRect() const { return _evfZoomRect; }
			EdsUInt32 getEvfAFMode() const { return _evfAFMode; }
			std::string getModelName() { return _modelName; }
			EdsFocusInfo getFocusInfo()const { return _focusInfo; }

			//List of value in which taking a picture parameter can be set
			EdsPropertyDesc getAEModeDesc() const { return _AEModeDesc; }
			EdsPropertyDesc getAvDesc() const { return _AvDesc; }
			EdsPropertyDesc getTvDesc()	const { return _TvDesc; }
			EdsPropertyDesc getIsoDesc()	const { return _IsoDesc; }
			EdsPropertyDesc getMeteringModeDesc()	const { return _MeteringModeDesc; }
			EdsPropertyDesc getExposureCompensationDesc()	const { return _ExposureCompensationDesc; }
			EdsPropertyDesc getImageQualityDesc()	const { return _ImageQualityDesc; }
			EdsPropertyDesc getEvfAFModeDesc()	const { return _evfAFModeDesc; }

			//List of value in which taking a picture parameter can be set
			void setAEModeDesc(const EdsPropertyDesc* desc) { 
				_AEModeDesc = *desc;  
				CI_LOG_D(deviceBodyId << " :: _AEModeDesc: " << CanonPropertyDescToString(desc)); 
			}
			void setAvDesc(const EdsPropertyDesc* desc) { 
				_AvDesc = *desc;  
				CI_LOG_D(deviceBodyId << " :: _AvDesc: " << CanonAvPropertyDescToString(desc)); 
			}
			void setTvDesc(const EdsPropertyDesc* desc) { 
				_TvDesc = *desc;  
				CI_LOG_D(deviceBodyId << " :: _TvDesc: " << CanonTvPropertyDescToString(desc)); 
			}
			void setIsoDesc(const EdsPropertyDesc* desc) { 
				_IsoDesc = *desc; 
				CI_LOG_D(deviceBodyId << " :: _IsoDesc: " << CanonIsoPropertyDescToString(desc)); 
			}
			void setMeteringModeDesc(const EdsPropertyDesc* desc) { _MeteringModeDesc = *desc; CI_LOG_D(deviceBodyId << " :: _MeteringModeDesc: " << CanonPropertyDescToString(desc)); }
			void setExposureCompensationDesc(const EdsPropertyDesc* desc) { _ExposureCompensationDesc = *desc; CI_LOG_D(deviceBodyId << " :: _ExposureCompensationDesc: " << CanonPropertyDescToString(desc)); }
			void setImageQualityDesc(const EdsPropertyDesc* desc) { _ImageQualityDesc = *desc; CI_LOG_D(deviceBodyId << " :: _ImageQualityDesc: " << CanonPropertyDescToString(desc)); }
			void setEvfAFModeDesc(const EdsPropertyDesc* desc) { _evfAFModeDesc = *desc; CI_LOG_D(deviceBodyId << " :: _evfAFModeDesc: " << CanonPropertyDescToString(desc)); }

		  public:
			//Setting of taking a picture parameter(UInt32)
			void setPropertyUInt32(EdsUInt32 propertyID, EdsUInt32 value){
				switch (propertyID)
				{
				case kEdsPropID_AEModeSelect:			setAEMode(value);					break;
				case kEdsPropID_Tv:						setTv(value);						break;
				case kEdsPropID_Av:						setAv(value);						break;
				case kEdsPropID_ISOSpeed:				setIso(value);						break;
				case kEdsPropID_MeteringMode:			setMeteringMode(value);				break;
				case kEdsPropID_ExposureCompensation:	setExposureCompensation(value);		break;
				case kEdsPropID_ImageQuality:			setImageQuality(value);				break;
				case kEdsPropID_Evf_Mode:				setEvfMode(value);					break;
				case kEdsPropID_Evf_OutputDevice:		setEvfOutputDevice(value);			break;
				case kEdsPropID_Evf_DepthOfFieldPreview:setEvfDepthOfFieldPreview(value);	break;
				case kEdsPropID_Evf_AFMode:				setEvfAFMode(value);				break;
				case kEdsPropID_AFMode:					setAFMode(value);					break;
				default:								CI_LOG_W(deviceBodyId << " :: unhandled property :: " << CanonPropertyToString(propertyID) << " :: " << std::to_string(value)); break;
				}
			}

			//Setting of taking a picture parameter(String)
			void setPropertyString(EdsUInt32 propertyID, EdsChar* str)
			{
				switch (propertyID)
				{
				case kEdsPropID_BodyIDEx:
					deviceBodyId = std::string(str);
					CI_LOG_I("Body ID :: " << deviceBodyId);
					break;
				case kEdsPropID_ProductName:
					CI_LOG_I("kEdsPropID_ProductName:" << str);
					setModelName(str);					
					break;
				default:
					CI_LOG_W(deviceBodyId << " :: unhandled property :: " << CanonPropertyToString(propertyID) << " :: " << str); 
					break;
				}
			}

			void setPropertyFocusInfo(EdsUInt32 propertyID, EdsFocusInfo info)
			{
				switch (propertyID)
				{
				case kEdsPropID_FocusInfo:				setFocusInfo(info);				break;
				}
			}

			//Setting of value list that can set taking a picture parameter
			void setPropertyDesc(EdsUInt32 propertyID, const EdsPropertyDesc* desc)
			{
				switch (propertyID)
				{
				case kEdsPropID_AEModeSelect:			setAEModeDesc(desc);				break;
				case kEdsPropID_Tv:						setTvDesc(desc);					break;
				case kEdsPropID_Av:						setAvDesc(desc);					break;
				case kEdsPropID_ISOSpeed:				setIsoDesc(desc);					break;
				case kEdsPropID_MeteringMode:			setMeteringModeDesc(desc);			break;
				case kEdsPropID_ExposureCompensation:	setExposureCompensationDesc(desc);	break;
				case kEdsPropID_ImageQuality:			setImageQualityDesc(desc);			break;
				case kEdsPropID_Evf_AFMode:				setEvfAFModeDesc(desc);				break;
				}
			}

			//Acquisition of value list that can set taking a picture parameter
			EdsPropertyDesc getPropertyDesc(EdsUInt32 propertyID)
			{
				EdsPropertyDesc desc = { 0 };
				switch (propertyID)
				{
				case kEdsPropID_AEModeSelect:			desc = getAEModeDesc();					break;
				case kEdsPropID_Tv:						desc = getTvDesc();						break;
				case kEdsPropID_Av:						desc = getAvDesc();						break;
				case kEdsPropID_ISOSpeed:				desc = getIsoDesc();					break;
				case kEdsPropID_MeteringMode:			desc = getMeteringModeDesc();			break;
				case kEdsPropID_ExposureCompensation:	desc = getExposureCompensationDesc();	break;
				case kEdsPropID_ImageQuality:			desc = getImageQualityDesc();			break;
				case kEdsPropID_Evf_AFMode:				desc = getEvfAFModeDesc();				break;
				}
				return desc;
			}
		};

	}
}
