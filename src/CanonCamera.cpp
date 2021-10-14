/*
 * CinderCanon.cpp
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
#include "cinder/Filesystem.h"

// using namespace canon;

EdsError EDSCALLBACK DownloadImageProgress(EdsUInt32 inPercent ,EdsVoid* inContext ,EdsBool* outCancel) {
	printf("Canon: downloading image: %d.\n", (int)inPercent);
	return EDS_ERR_OK;
};

bool ci::canon::CanonCamera::isSDKLoaded = false;

ci::canon::CanonCamera::CanonCamera() : bCameraIsConnected( false ), bIsLiveView( false ) {}

void ci::canon::CanonCamera::setup( int cameraIndex ) {

	if (bCameraIsConnected) {
		CI_LOG_E("CAMERA IS ALREADY SET UP AND CONNECTED");
		return;
	}

	EdsError err = EDS_ERR_OK;

	// Initialize sdk
	if (!ci::canon::CanonCamera::isSDKLoaded) {
		err = EdsInitializeSDK();
		if (err != EDS_ERR_OK) { 
			CI_LOG_E("Cinder-Canon :: Couldn't initialize SDK"); 
		} else {
			ci::canon::CanonCamera::isSDKLoaded = true;
		}
	}
	
	// Get Camera List and open a session
	err = EdsGetCameraList( &mCamera_list );
	if (err != EDS_ERR_OK) { CI_LOG_E("Cinder-Canon :: Couldn't retrieve camera list"); }
	
	// console() << "Cinder-Canon :: initialized.  Found " << getNumConnectedCameras() << " camera(s) connected" << endl;
	
	if( getNumConnectedCameras() == 0 ) return;
	
	err = EdsGetChildAtIndex(mCamera_list, cameraIndex, &mCamera );

	if (err != EDS_ERR_OK) { CI_LOG_E("Cinder-Canon :: Couldn't retrieve camera from list"); }

	setup(mCamera, cameraIndex);
}

void ci::canon::CanonCamera::setup(EdsCameraRef cameraRef, int cameraIndex) {
	if (bCameraIsConnected) {
		CI_LOG_E("CAMERA IS ALREADY SET UP AND CONNECTED");
		return;
	}
	mCamera = cameraRef;
	deviceIndex = cameraIndex;

	EdsError err = EDS_ERR_OK;
	getDeviceInfo(mCamera);

	try {
		err = EdsOpenSession(mCamera);
		if (err != EDS_ERR_OK) { 
			CI_LOG_E("Couldn't open camera session"); 
			return;
		}
	}
	catch (...) {
		CI_LOG_E("EDS OPEN SESSION FAILED.");
		return;
	}

	bCameraIsConnected = true;

	// Set event listener
	EdsSetObjectEventHandler(mCamera, kEdsObjectEvent_All, ci::canon::CanonCamera::handleObjectEvent, (EdsVoid*)this);
	EdsSetPropertyEventHandler(mCamera, kEdsPropertyEvent_All, ci::canon::CanonCamera::handlePropertyEvent, (EdsVoid*)this);
	EdsSetCameraStateEventHandler(mCamera, kEdsStateEvent_All, ci::canon::CanonCamera::handleStateEvent, (EdsVoid*)this);

	//once session is open get the serial
	getPropertyFromCamera(kEdsPropID_BodyIDEx);
	//storeDeviceSerial(mCamera);
	
	bFrameNew = false;
//	mLivePixels = Surface8u::create(1024, 680, false, SurfaceChannelOrder::RGB); // what about this resolution?

	//err = getPropertyDescFromCamera(kEdsPropID_ISOSpeed);
	//err = setProperty(kEdsPropID_ISOSpeed, 0x7d);
	//lockUI();
}

int ci::canon::CanonCamera::getNumConnectedCameras() {
	EdsUInt32 numCameras = 0;
	EdsGetChildCount(mCamera_list, &numCameras);
	return numCameras;
}

//void CinderCanon::storeDeviceSerial(EdsCameraRef cam) {
//	if (!isCameraConnected()) {
//		CI_LOG_E(deviceBodyId << " NOT CONNECTED");
//		return;
//	}
//	EdsError err = EDS_ERR_OK;
//	//EdsDataType dataType;
//	//EdsUInt32 dataSize;
//	//EdsGetPropertySize(cam, kEdsPropID_BodyIDEx, 0, &dataType, &dataSize);
//	//console() << "id size"<< dataSize << endl;
//	//buffer is actually much bigger
//	//like 4274327 or something
//	char buf[800];
//	err = EdsGetPropertyData(cam, kEdsPropID_BodyIDEx, 0, 800, &buf);
//	if (err != EDS_ERR_OK) {
//		CI_LOG_E("Cinder-Canon :: Couldn't retrieve Body ID");
//	}
//	else {
//		std::string bodyId(buf);
//		deviceBodyId = bodyId;
//		console() << "Cinder-Canon :: Body ID :: " << bodyId << endl;
//	}
//}

void ci::canon::CanonCamera::getDeviceInfo( EdsCameraRef cam ) {
	EdsError err = EDS_ERR_OK;
	EdsDeviceInfo info;
	err = EdsGetDeviceInfo( cam, &info );
	if( err != EDS_ERR_OK ){ CI_LOG_E("Cinder-Canon :: Couldn't retrieve camera info"); }
	CI_LOG_I("Cinder-Canon :: Device name :: " << info.szDeviceDescription);
}

EdsError ci::canon::CanonCamera::lockUI() {
	EdsError err = EdsSendStatusCommand(mCamera, kEdsCameraStatusCommand_UILock, 0);
	if (err == EDS_ERR_OK) {
		CI_LOG_D(deviceBodyId << " :: UI locked.");
		mUILocked = true;
	}
	return err;
}
EdsError ci::canon::CanonCamera::unlockUI() {
	EdsError err = EdsSendStatusCommand(mCamera, kEdsCameraStatusCommand_UIUnLock, 0);
	if (err == EDS_ERR_OK) {
		CI_LOG_D("UI unlocked.");
		mUILocked = false;
	}
	return err;
}

bool ci::canon::CanonCamera::takePicture() //PhotoHandler * photoHandler)
{
	if (!isCameraConnected()) {
		CI_LOG_E(deviceBodyId << " NOT CONNECTED");
		return false;
	}
	
	CI_LOG_I( deviceBodyId << " :: Attempting to take picture ------------------------------------------------------");

	bool success = sendCommand(mCamera, kEdsCameraCommand_TakePicture, 0);
	
	//bool success = sendCommand(mCamera, kEdsCameraCommand_PressShutterButton, kEdsCameraCommand_ShutterButton_Completely_NonAF);
	//bool success = sendCommand(mCamera, kEdsCameraCommand_PressShutterButton, kEdsCameraCommand_ShutterButton_Completely);
	//sendCommand(mCamera, kEdsCameraCommand_PressShutterButton, kEdsCameraCommand_ShutterButton_OFF);
	return success;
}

void ci::canon::CanonCamera::startLiveView()
{
	if (!isCameraConnected()) {
		CI_LOG_E(deviceBodyId << " NOT CONNECTED");
		return;
	}
	CI_LOG_D( deviceBodyId << " :: start live view");
	EdsError err = EDS_ERR_OK;
	
	// Get the output device for the live view image
	EdsUInt32 device;
	err = EdsGetPropertyData(mCamera, kEdsPropID_Evf_OutputDevice, 0, sizeof(device), &device );
	
	// PC live view starts by setting the PC as the output device for the live view image.
	if(err == EDS_ERR_OK)
	{
		device |= kEdsEvfOutputDevice_PC;
		err = EdsSetPropertyData(mCamera, kEdsPropID_Evf_OutputDevice, 0 , sizeof(device), &device);
		bIsLiveView = true;
	}
	
	// A property change event notification is issued from the camera if property settings are made successfully.
	// Start downloading of the live view image once the property change notification arrives.
}

void ci::canon::CanonCamera::endLiveView()
{
	if (!isCameraConnected()) {
		CI_LOG_E(deviceBodyId << " NOT CONNECTED");
		return;
	}
	if (!bIsLiveView) {
		//CI_LOG_D(deviceBodyId << " LiveView not running");
		return;
	}
	EdsError err = EDS_ERR_OK;
	
	// Get the output device for the live view image
	EdsUInt32 device;
	err = EdsGetPropertyData(mCamera, kEdsPropID_Evf_OutputDevice, 0, sizeof(device), &device );
	
	// PC live view ends if the PC is disconnected from the live view image output device.
	if(err == EDS_ERR_OK)
	{
		device &= ~kEdsEvfOutputDevice_PC;
		err = EdsSetPropertyData(mCamera, kEdsPropID_Evf_OutputDevice, 0 , sizeof(device), &device);
	}
	
	bIsLiveView = false;
}

EdsError ci::canon::CanonCamera::downloadEvfData( EdsCameraRef camera )
{
	if( !bIsLiveView ){
		CI_LOG_W("LiveView was not running. Starting LiveView");
		startLiveView();
		if (bIsLiveView) return EDS_ERR_OK;
		else return EDS_ERR_COMM_DISCONNECTED;
	}
	
	EdsError err = EDS_ERR_OK;
	EdsStreamRef stream = NULL;
	EdsEvfImageRef evfImage = NULL;
	
	// Create memory stream.
	err = EdsCreateMemoryStream( 0, &stream);
	
	// Create EvfImageRef.
	if(err == EDS_ERR_OK) {
		err = EdsCreateEvfImageRef(stream, &evfImage);
	}
	
	// Download live view image data.
	if(err == EDS_ERR_OK){
		err = EdsDownloadEvfImage(camera, evfImage);
	}
	
	// Get the incidental data of the image.
	if(err == EDS_ERR_OK){
		// Get the zoom ratio
		EdsUInt32 zoom;
		EdsGetPropertyData(evfImage, kEdsPropID_Evf_ZoomPosition, 0, sizeof(zoom), &zoom );
		
		// Get the focus and zoom border position
		EdsPoint point;
		EdsGetPropertyData(evfImage, kEdsPropID_Evf_ZoomPosition, 0 , sizeof(point), &point);
	}
	
	// Display image
	EdsUInt64 length;
	unsigned char* image_data;
	EdsGetLength( stream, &length );
	if( length <= 0 ) return EDS_ERR_OK;

	EdsGetPointer( stream, (EdsVoid**)&image_data );
	
	// reserve memory
	BufferRef bufferRef = Buffer::create( image_data, length );
	mLivePixels = Surface8u::create( loadImage( DataSourceBuffer::create(bufferRef), ImageSource::Options(), "jpg" ) );
	bFrameNew = true;
	
	// Release stream
	if(stream != NULL) {
		EdsRelease(stream);
		stream = NULL;
	}
	// Release evfImage
	if(evfImage != NULL) {
		EdsRelease(evfImage);
		evfImage = NULL;
	}
	
	return EDS_ERR_OK;
}

void ci::canon::CanonCamera::downloadImage(EdsDirectoryItemRef dirItem) //, PhotoHandler * photoHandler)
{
	if (!isCameraConnected()) {
		CI_LOG_E(deviceBodyId << " NOT CONNECTED");
		return;
	}

	//lockUI();
	EdsError err = EDS_ERR_OK;
	EdsStreamRef stream = NULL;

	// Get info about new image.
	EdsDirectoryItemInfo dir_item_info;
	err = EdsGetDirectoryItemInfo(dirItem, &dir_item_info);
	if(err != EDS_ERR_OK) {
		printf("Canon: error while trying to get more info about image to be downloaded.\n");
	}
	
	// Created file stream to download image.
	std::string downloadDest = getPhotoDownloadDirectory(); // "/tmp/canon";

	fs::path downloadPath = fs::path(downloadDest) / std::string(dir_item_info.szFileName);

	if(err == EDS_ERR_OK) {
		err = EdsCreateFileStream(downloadPath.generic_string().c_str(), kEdsFileCreateDisposition_CreateAlways, kEdsAccess_ReadWrite, &stream);
	}
	if(err != EDS_ERR_OK) {
		CI_LOG_E("Canon: error while creating download stream.");
	}
	
	// Set progress
	if(err == EDS_ERR_OK) {
		err = EdsSetProgressCallback(stream, DownloadImageProgress, kEdsProgressOption_Periodically, this);
	}
	if(err != EDS_ERR_OK) {
		CI_LOG_E("Canon: error while setting download progress function.");
	}
	
	// Download image.
	if(err == EDS_ERR_OK) {
		err = EdsDownload(dirItem, dir_item_info.size, stream);
	}
	if(err != EDS_ERR_OK) {
		CI_LOG_E("Canon: error while downloading item.");
	}
	
	// Tell we're ready.
	if(err == EDS_ERR_OK) {        
		CI_LOG_D("Downloaded image to " << downloadPath.generic_string());
		err = EdsDownloadComplete(dirItem);
	}
	if(err != EDS_ERR_OK) {
		CI_LOG_E("error while telling we're ready with the file download.");
	}

	if (deleteAfterDownload) {
		//delete the image
		err = EDS_ERR_OK;
		err = EdsDeleteDirectoryItem(dirItem);
		if (err != EDS_ERR_OK) {
			CI_LOG_E("error deleting file");
		}
	}
	
	// Release dir item.
	/*
	if(dirItem != NULL) {
		err = EdsRelease(dirItem);
		if(err != EDS_ERR_OK) {
			printf("Canon: error releasing dir item when downloading.\n");
		}
		dirItem = NULL;
	}
	*/
	
	// Release stream
	if(stream != NULL) {
		err = EdsRelease(stream);
		if(err != EDS_ERR_OK) {
			CI_LOG_E("Canon: error while releasing download stream.");
		}
		stream = NULL;
	}
	
	if (err != EDS_ERR_OK)
	{
		downloadDest = "";
	}

	/*if (photoHandler)
	{
		photoHandler->photoDownloaded(downloadPath.generic_string(), err);
	}*/
	//unlockUI();
	photoDownloaded(downloadPath.generic_string(), err);
}

EdsError EDSCALLBACK ci::canon::CanonCamera::handleObjectEvent(EdsUInt32 inEvent, EdsBaseRef inRef, EdsVoid* inContext )
{
	ci::canon::CanonCamera* cc = (ci::canon::CanonCamera*)inContext;
    if(cc->logHandleEvents) CI_LOG_D( cc->deviceBodyId << " :: Object Event :: " << CanonEventToString(inEvent));

	switch(inEvent) {
		//case kEdsObjectEvent_DirItemRequestTransfer:
		case kEdsObjectEvent_DirItemCreated:
		{
			CI_LOG_D(cc->deviceBodyId << " :: Object Event :: Photo Taken. Calling photoTaken");
			EdsDirectoryItemRef dirItem = (EdsDirectoryItemRef)inRef;
			cc->photoTaken(dirItem, EDS_ERR_OK);
			break;
		}
	}
	
	if(inRef != NULL) {
		EdsRelease(inRef);
	}

	return EDS_ERR_OK;
}

EdsError EDSCALLBACK ci::canon::CanonCamera::handlePropertyEvent( EdsUInt32 inEvent, EdsUInt32 inPropertyID, EdsUInt32 inParam, EdsVoid* inContext ) {
	ci::canon::CanonCamera* cc = (ci::canon::CanonCamera*)inContext;
    if(cc->logHandleEvents) CI_LOG_D(cc->deviceBodyId << " :: Property Event :: " << CanonEventToString(inEvent) << " :: Property: " << CanonPropertyToString(inPropertyID)); // << ", param:" << std::to_string(inParam) );
	
	if( inPropertyID == kEdsPropID_Evf_OutputDevice ){
		CI_LOG_D(cc->deviceBodyId << " :: ready for live viewing");
	}
	switch (inEvent) {
		case kEdsPropertyEvent_PropertyChanged:
			if (cc) cc->getPropertyFromCamera(inPropertyID);
			break;

		case kEdsPropertyEvent_PropertyDescChanged:
			if (cc) cc->getPropertyDescFromCamera(inPropertyID);
			break;
	}
	
	return EDS_ERR_OK;
}

EdsError EDSCALLBACK ci::canon::CanonCamera::handleStateEvent(EdsUInt32 inEvent, EdsUInt32 inParam, EdsVoid* inContext) {
	ci::canon::CanonCamera* cc = (ci::canon::CanonCamera*)inContext;
    if(cc->logHandleEvents) CI_LOG_D(cc->deviceBodyId << " :: State Event :: " << CanonEventToString(inEvent));

	switch(inEvent) {
		case kEdsStateEvent_Shutdown: {
			cc->onCameraDisconnected();
			break;
		}
		default:
			break;
	};
	return EDS_ERR_OK;
}

bool ci::canon::CanonCamera::sendCommand( EdsCameraRef inCameraRef, EdsUInt32 inCommand, EdsUInt32 inParam ) {
	if (!isCameraConnected()) {
		CI_LOG_E(deviceBodyId << " :: NOT CONNECTED");
		return false;
	}
	EdsError err = EDS_ERR_OK;
	err = EdsSendCommand( inCameraRef, inCommand, inParam );
	if(err != EDS_ERR_OK) {
		CI_LOG_E(deviceBodyId << " :: error while sending command "<< err << " " <<  CanonErrorToString(err) << ".");
		if (err == EDS_ERR_INVALID_HANDLE) {
			onCameraDisconnected();
			return false;
		}
		if(err == EDS_ERR_DEVICE_BUSY) {
			CI_LOG_E(deviceBodyId << " :: device busy.");
			return false;
		}
	}
	return true;
}

EdsError ci::canon::CanonCamera::getPropertyFromCamera(EdsPropertyID propertyID, EdsInt32  inParam){
	if (!isCameraConnected()) {
		CI_LOG_E(deviceBodyId << " :: NOT CONNECTED");
		return EDS_ERR_COMM_DISCONNECTED;
	}

	EdsError err = EDS_ERR_OK;
	EdsDataType	dataType = kEdsDataType_Unknown;
	EdsUInt32   dataSize = 0;

	if (logGetPropertyDescFromCamera) CI_LOG_D(deviceBodyId << " :: " << CanonPropertyToString(propertyID) << " :: param: " << std::to_string(inParam));

	if (propertyID == kEdsPropID_Unknown) {
		CI_LOG_D(deviceBodyId << " :: propertyID == kEdsPropID_Unknown :: If unknown is returned for the property ID , the required property must be retrieved again");
		//If unknown is returned for the property ID , the required property must be retrieved again
		if (err == EDS_ERR_OK) err = getPropertyFromCamera(kEdsPropID_AEModeSelect, inParam);
		if (err == EDS_ERR_OK) err = getPropertyFromCamera(kEdsPropID_Tv, inParam);
		if (err == EDS_ERR_OK) err = getPropertyFromCamera(kEdsPropID_Av, inParam);
		if (err == EDS_ERR_OK) err = getPropertyFromCamera(kEdsPropID_ISOSpeed, inParam);
		if (err == EDS_ERR_OK) err = getPropertyFromCamera(kEdsPropID_MeteringMode, inParam);
		if (err == EDS_ERR_OK) err = getPropertyFromCamera(kEdsPropID_ExposureCompensation, inParam);
		if (err == EDS_ERR_OK) err = getPropertyFromCamera(kEdsPropID_ImageQuality, inParam);

		return err;
	}

	//Acquisition of the property size
	if (err == EDS_ERR_OK) {
		err = EdsGetPropertySize(mCamera, propertyID, inParam, &dataType, &dataSize);
	}

	if (err == EDS_ERR_OK) {
		if (dataType == kEdsDataType_UInt32) {
			EdsUInt32 data;
			//Acquisition of the property
			err = EdsGetPropertyData(mCamera, propertyID, inParam, dataSize, &data);
			/*if (propertyID == kEdsPropID_CFn && err == EDS_ERR_OK) {
				CI_LOG_D(deviceBodyId << " :: " << CanonPropertyToString(propertyID) << ":: data: " << data);
			}*/
			//Acquired property value is set
			if (err == EDS_ERR_OK ) { setPropertyUInt32(propertyID, data); }
		} else 
		if (dataType == kEdsDataType_String) {
			EdsChar str[EDS_MAX_NAME];
			//Acquisition of the property
			err = EdsGetPropertyData(mCamera, propertyID, 0, dataSize, str);
			//Acquired property value is set
			if (err == EDS_ERR_OK) { setPropertyString(propertyID, str);}
		} else 
		if (dataType == kEdsDataType_FocusInfo) {
			EdsFocusInfo focusInfo;
			//Acquisition of the property
			err = EdsGetPropertyData(mCamera, propertyID, 0, dataSize, &focusInfo);
			//Acquired property value is set
			if (err == EDS_ERR_OK){ setFocusInfo(focusInfo);}
		}
		else {
			CI_LOG_W(deviceBodyId << " :: unhandled dataType for " << CanonPropertyToString(propertyID));
		}
	}
	else {
		CI_LOG_W(deviceBodyId << " :: Warning - could not get PropertySize :: " << CanonErrorToString(err));
	}

	if (err != EDS_ERR_OK) {
		CI_LOG_W(deviceBodyId << " :: Warning - could not receive property data :: " << CanonErrorToString(err));
	}

	//Update notification
	return err;
}

EdsError ci::canon::CanonCamera::getPropertyDescFromCamera(EdsPropertyID propertyID)
{
	if (!isCameraConnected()) {
		CI_LOG_E(deviceBodyId << " :: NOT CONNECTED");
		return EDS_ERR_COMM_DISCONNECTED;
	}

	EdsError  err = EDS_ERR_OK;
	EdsPropertyDesc	 propertyDesc = { 0 };

	CI_LOG_D(deviceBodyId << " :: " << CanonPropertyToString(propertyID));

	if (propertyID == kEdsPropID_Unknown) {
		//If unknown is returned for the property ID , the required property must be retrieved again
		CI_LOG_D(deviceBodyId << " :: propertyID == kEdsPropID_Unknown :: If unknown is returned for the property ID , the required property must be retrieved again");
		if (err == EDS_ERR_OK) err = getPropertyDescFromCamera(kEdsPropID_AEModeSelect);
		if (err == EDS_ERR_OK) err = getPropertyDescFromCamera(kEdsPropID_Tv);
		if (err == EDS_ERR_OK) err = getPropertyDescFromCamera(kEdsPropID_Av);
		if (err == EDS_ERR_OK) err = getPropertyDescFromCamera(kEdsPropID_ISOSpeed);
		if (err == EDS_ERR_OK) err = getPropertyDescFromCamera(kEdsPropID_MeteringMode);
		if (err == EDS_ERR_OK) err = getPropertyDescFromCamera(kEdsPropID_ExposureCompensation);
		if (err == EDS_ERR_OK) err = getPropertyDescFromCamera(kEdsPropID_ImageQuality);
		return err;
	}

	//Acquisition of value list that can be set
	if (err == EDS_ERR_OK){
		err = EdsGetPropertyDesc(mCamera, propertyID, &propertyDesc);
	}

	//The value list that can be the acquired setting it is set		
	if (err == EDS_ERR_OK){
		setPropertyDesc(propertyID, &propertyDesc);
	}

	if (err != EDS_ERR_OK) {
		CI_LOG_W(deviceBodyId << " :: Warning :: " << CanonErrorToString(err));
	}

	return err;
}

void ci::canon::CanonCamera::onCameraDisconnected() {
	CI_LOG_I(deviceBodyId << " :: " << deviceIndex << " :: onCameraDisconnected ");
	bCameraIsConnected = false;
	bIsLiveView = false;
	EdsSetObjectEventHandler(mCamera, kEdsObjectEvent_All, NULL, (EdsVoid*)this);
	EdsSetPropertyEventHandler(mCamera, kEdsPropertyEvent_All, NULL, (EdsVoid*)this);
	EdsSetCameraStateEventHandler(mCamera, kEdsStateEvent_All, NULL, (EdsVoid*)this);
	EdsCloseSession(mCamera);
	EdsRelease(mCamera);
	EdsRelease(mCamera_list);
	deviceIndex = -1;
	signal_disconnected.emit(deviceBodyId);
}


void ci::canon::CanonCamera::cleanup(bool terminateSdk)
{
	CI_LOG_I(deviceBodyId << " :: cleanup");
	bCameraIsConnected = false;
	bIsLiveView = false;
	unlockUI();
	EdsSetObjectEventHandler(mCamera, kEdsObjectEvent_All, NULL, (EdsVoid*)this);
	EdsSetPropertyEventHandler(mCamera, kEdsPropertyEvent_All, NULL, (EdsVoid*)this);
	EdsSetCameraStateEventHandler(mCamera, kEdsStateEvent_All, NULL, (EdsVoid*)this);
	EdsCloseSession( mCamera );
	EdsRelease(mCamera);
	EdsRelease(mCamera_list);
	deviceIndex = -1;
	if (terminateSdk) {
		EdsError err = EDS_ERR_OK;
		err = EdsTerminateSDK();
		if (err == EDS_ERR_OK) {
			ci::canon::CanonCamera::isSDKLoaded = false;
		}
		else {
			CI_LOG_E( deviceBodyId << " :: error terminating sdk :: " << CanonErrorToString(err));
		}
	}
}

#pragma mark
#pragma mark Callbacks from the canon SDK
//-----------------------------------------------------------------------------------

void ci::canon::CanonCamera::photoTaken(EdsDirectoryItemRef directoryItem, EdsError error){
	signal_photoTaken.emit(deviceBodyId);
	if (autoDownload) downloadImage(directoryItem); // , this);
}

void ci::canon::CanonCamera::photoDownloaded(const std::string& downloadPath, EdsError error){
	signal_photoDownloaded.emit(deviceBodyId, downloadPath);
}

// Delegate method to tell the canon where to download the full-res image.
std::string ci::canon::CanonCamera::getPhotoDownloadDirectory(){
	return downloadDirectory;
}

