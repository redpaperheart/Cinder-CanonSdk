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

#include "CinderCanon.h"
#include "cinder/Filesystem.h"

using namespace canon;

EdsError EDSCALLBACK DownloadImageProgress(EdsUInt32 inPercent ,EdsVoid* inContext ,EdsBool* outCancel) {
	printf("Canon: downloading image: %d.\n", (int)inPercent);
	return EDS_ERR_OK;
};

bool CinderCanon::isSDKLoaded = false;

CinderCanon::CinderCanon() : bCameraIsConnected( false ), bIsLiveView( false ) {}

void CinderCanon::setup( int cameraIndex ) {

	if (bCameraIsConnected) {
		CI_LOG_E("CAMERA IS ALREADY SET UP AND CONNECTED");
		return;
	}

	EdsError err = EDS_ERR_OK;

	// Initialize sdk
	if (!CinderCanon::isSDKLoaded) {
		err = EdsInitializeSDK();
		if (err != EDS_ERR_OK) { 
			CI_LOG_E("Cinder-Canon :: Couldn't initialize SDK"); 
		} else {
			CinderCanon::isSDKLoaded = true;
		}
	}
	
	// Get Camera List and open a session
	err = EdsGetCameraList( &mCamera_list );
	if (err != EDS_ERR_OK) { CI_LOG_E("Cinder-Canon :: Couldn't retrieve camera list"); }
	
	// console() << "Cinder-Canon :: initialized.  Found " << getNumConnectedCameras() << " camera(s) connected" << endl;
	
	if( getNumConnectedCameras() == 0 ) return;
	
	err = EdsGetChildAtIndex(mCamera_list, cameraIndex, &mCamera );

	if (err != EDS_ERR_OK) { CI_LOG_E("Cinder-Canon :: Couldn't retrieve camera from list"); }

	setup(mCamera);
}

void CinderCanon::setup(EdsCameraRef cameraRef) {
	if (bCameraIsConnected) {
		CI_LOG_E("CAMERA IS ALREADY SET UP AND CONNECTED");
		return;
	}
	mCamera = cameraRef;

	EdsError err = EDS_ERR_OK;
	getDeviceInfo(mCamera);

	try {
		err = EdsOpenSession(mCamera);
		if (err != EDS_ERR_OK) { console() << "Cinder-Canon :: Couldn't open camera session" << endl; }
	}
	catch (...) {
		CI_LOG_E("EDS OPEN SESSION FAILED.");
		return;
	}

	bCameraIsConnected = true;

	// Set event listener
	EdsSetObjectEventHandler(mCamera, kEdsObjectEvent_All, CinderCanon::handleObjectEvent, (EdsVoid*)this);
	EdsSetPropertyEventHandler(mCamera, kEdsPropertyEvent_All, CinderCanon::handlePropertyEvent, (EdsVoid*)this);
	EdsSetCameraStateEventHandler(mCamera, kEdsStateEvent_All, CinderCanon::handleStateEvent, (EdsVoid*)this);

	//once session is open get the serial
	getPropertyFromCamera(kEdsPropID_BodyIDEx);
	//storeDeviceSerial(mCamera);
	
	bFrameNew = false;
	mLivePixels = Surface8u(1024, 680, false, SurfaceChannelOrder::RGB); // what about this resolution?

	//err = getPropertyDescFromCamera(kEdsPropID_ISOSpeed);
	//err = setProperty(kEdsPropID_ISOSpeed, 0x7d);
}

int CinderCanon::getNumConnectedCameras() {
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
void CinderCanon::getDeviceInfo( EdsCameraRef cam ) {
	EdsError err = EDS_ERR_OK;
	EdsDeviceInfo info;
	err = EdsGetDeviceInfo( cam, &info );
	if( err != EDS_ERR_OK ){ CI_LOG_E("Cinder-Canon :: Couldn't retrieve camera info"); }
	CI_LOG_I("Cinder-Canon :: Device name :: " << info.szDeviceDescription);
}

bool CinderCanon::takePicture() //PhotoHandler * photoHandler)
{
	if (!isCameraConnected()) {
		CI_LOG_E(deviceBodyId << " NOT CONNECTED");
		return false;
	}
	
	console() << "Cinder-Canon :: ------------------------------------------------------ " << endl;
	console() << "Cinder-Canon :: " << deviceBodyId << " :: Attempting to take picture " << endl;
	//bool success = true;
	/*if (!sendCommand(mCamera, kEdsCameraStatusCommand_UILock, 0)) success = false;
	std::string s_str = (success) ? "success" : "failed";
	console() << "Cinder-Canon :: " << deviceBodyId << " :: cam locked: "<< s_str << endl;
	if (success && sendCommand(mCamera, kEdsCameraCommand_TakePicture, 0)) {
		console() << "Cinder-Canon :: " << deviceBodyId << " :: take picture success" << endl;
	}
	else {
		console() << "Cinder-Canon :: " << deviceBodyId << " :: take picture FAILED. cam locked: " << s_str << endl;
	}*/

	bool success = sendCommand(mCamera, kEdsCameraCommand_PressShutterButton, kEdsCameraCommand_ShutterButton_Completely);
	sendCommand(mCamera, kEdsCameraCommand_PressShutterButton, kEdsCameraCommand_ShutterButton_OFF);

	return success;
}

void CinderCanon::startLiveView()
{
	if (!isCameraConnected()) {
		CI_LOG_E(deviceBodyId << " NOT CONNECTED");
		return;
	}
	console() << "Cinder-Canon :: " << deviceBodyId << " :: start live view" << endl;
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

void CinderCanon::endLiveView()
{
	if (!isCameraConnected()) {
		CI_LOG_E(deviceBodyId << " NOT CONNECTED");
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

EdsError CinderCanon::downloadEvfData( EdsCameraRef camera )
{
	if( !bIsLiveView ){
		console() << "No live view" << endl;
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
//    Buffer buffer( image_data, length );
//    mLivePixels = Surface( loadImage( DataSourceBuffer::create(buffer), ImageSource::Options(), "jpg" ) );
	BufferRef bufferRef = Buffer::create( image_data, length );
	mLivePixels = Surface( loadImage( DataSourceBuffer::create(bufferRef), ImageSource::Options(), "jpg" ) );
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

void CinderCanon::downloadImage(EdsDirectoryItemRef dirItem) //, PhotoHandler * photoHandler)
{
	if (!isCameraConnected()) {
		CI_LOG_E(deviceBodyId << " NOT CONNECTED");
		return;
	}
	EdsError err = EDS_ERR_OK;
	EdsStreamRef stream = NULL;

	// Get info about new image.
	EdsDirectoryItemInfo dir_item_info;
	err = EdsGetDirectoryItemInfo(dirItem, &dir_item_info);
	if(err != EDS_ERR_OK) {
		printf("Canon: error while trying to get more info about image to be downloaded.\n");
	}
	
	// Created file stream to download image.
	string downloadDest = photoDownloadDirectory(); // "/tmp/canon";
	/*if (photoHandler)
	{
		downloadDest = photoHandler->photoDownloadDirectory();
	}*/

	fs::path downloadPath = fs::path(downloadDest) / string(dir_item_info.szFileName);

	if(err == EDS_ERR_OK) {
		err = EdsCreateFileStream(downloadPath.generic_string().c_str(),
								  kEdsFileCreateDisposition_CreateAlways,
								  kEdsAccess_ReadWrite,
								  &stream);
	}
	if(err != EDS_ERR_OK) {
		printf("Canon: error while creating download stream.\n");
	}
	
	// Set progress
	if(err == EDS_ERR_OK) {
		err = EdsSetProgressCallback(stream, DownloadImageProgress, kEdsProgressOption_Periodically, this);
	}
	if(err != EDS_ERR_OK) {
		printf("Canon: error while setting download progress function.\n");
	}
	
	// Download image.
	if(err == EDS_ERR_OK) {
		err = EdsDownload(dirItem, dir_item_info.size, stream);
	}
	if(err != EDS_ERR_OK) {
		printf("Canon: error while downloading item.\n");
	}
	
	// Tell we're ready.
	if(err == EDS_ERR_OK) {        
		console() << "Downloaded image to " << downloadPath.generic_string() << "\n";
		err = EdsDownloadComplete(dirItem);
	}
	if(err != EDS_ERR_OK) {
		printf("Canon: error while telling we're ready with the file download.\n");
	}

	if (deleteAfterDownload) {
		//delete the image
		err = EDS_ERR_OK;
		err = EdsDeleteDirectoryItem(dirItem);
		if (err != EDS_ERR_OK) {
			console() << "Cinder-Canon :: error deleting file" << endl;
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
			printf("Canon: error while releasing download stream.\n");
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
	photoDownloaded(downloadPath.generic_string(), err);
}

EdsError EDSCALLBACK CinderCanon::handleObjectEvent(EdsUInt32 inEvent, EdsBaseRef inRef, EdsVoid* inContext )
{
	CinderCanon* cc = (CinderCanon*)inContext;
    console() << "Cinder-Canon :: " << cc->deviceBodyId << " :: Object Event :: " << CanonEventToString(inEvent) << endl;

	switch(inEvent) {
		//case kEdsObjectEvent_DirItemRequestTransfer:
		case kEdsObjectEvent_DirItemCreated:
		{
			//if (SingletonPhotoHandler)
			//{
				console() << "Photo Taken. Calling photoTaken" << std::endl;
				EdsDirectoryItemRef dirItem = (EdsDirectoryItemRef)inRef;
				// NOTE: This is only called on success.
				// It should also be called on failure.
				//SingletonPhotoHandler->photoTaken(dirItem, EDS_ERR_OK);
				cc->photoTaken(dirItem, EDS_ERR_OK);
			//}
			//else {
				//console() << "No photo callback. Ignoring." << endl;
				// This downloads to /tmp/canon
				// ((CinderCanon *)inContext)->downloadImage(inRef, NULL);
			//}
			break;
		}
	}
	
	if(inRef != NULL) {
		EdsRelease(inRef);
	}

	return EDS_ERR_OK;
}

EdsError EDSCALLBACK CinderCanon::handlePropertyEvent( EdsUInt32 inEvent, EdsUInt32 inPropertyID, EdsUInt32 inParam, EdsVoid* inContext ) {
	CinderCanon* cc = (CinderCanon*)inContext;
	console() << "Cinder-Canon :: " << cc->deviceBodyId << " ::  Property Event :: " << CanonEventToString(inEvent) << " :: " << CanonPropertyToString(inPropertyID) << endl;
	
	if( inPropertyID == kEdsPropID_Evf_OutputDevice ){
		console() << "Cinder-Canon :: ready for live viewing" << endl;
	}
	switch (inEvent) {
		case kEdsPropertyEvent_PropertyChanged:
			//fireEvent(controller, "get_Property", &inPropertyID);
			if (cc) cc->getPropertyFromCamera(inPropertyID);
			break;

		case kEdsPropertyEvent_PropertyDescChanged:
			//fireEvent(controller, "get_PropertyDesc", &inPropertyID);
			break;
	}
	
	return EDS_ERR_OK;
}

EdsError EDSCALLBACK CinderCanon::handleStateEvent(EdsUInt32 inEvent, EdsUInt32 inParam, EdsVoid* inContext) {
	CinderCanon* cc = (CinderCanon*)inContext;
	console() << "Cinder-Canon :: " << cc->deviceBodyId << " :: State Event :: " << CanonEventToString(inEvent) << endl;

	switch(inEvent) {
			// Connection with camera lost (maybe power off?)
		case kEdsStateEvent_Shutdown: {
//			shutdown();
			cc->onCameraDisconnected();
			break;
		}
		default:
			break;
	};
	//printf("Cinder-Canon: handleStateEvent '%s'.\n", CanonEventToString(inEvent));
	
	return EDS_ERR_OK;
}

bool CinderCanon::sendCommand( EdsCameraRef inCameraRef, EdsUInt32 inCommand, EdsUInt32 inParam )
{
	if (!isCameraConnected()) {
		CI_LOG_E(deviceBodyId << " NOT CONNECTED");
		return false;
	}
	EdsError err = EDS_ERR_OK;
	err = EdsSendCommand( inCameraRef, inCommand, inParam );
	if(err != EDS_ERR_OK) {
		console() << "Cinder-Canon :: " << deviceBodyId << " :: error while sending command "<< err << " " <<  CanonErrorToString(err) << "." << endl;
		if (err == EDS_ERR_INVALID_HANDLE) {
			onCameraDisconnected();
			return false;
		}
		if(err == EDS_ERR_DEVICE_BUSY) {
			console() << "Cinder-Canon :: " << deviceBodyId << " :: device busy." << endl;
			return false;
		}
	}
	return true;
}

EdsError CinderCanon::getPropertyFromCamera(EdsPropertyID propertyID){
	if (!isCameraConnected()) {
		CI_LOG_E(deviceBodyId << " NOT CONNECTED");
		return EDS_ERR_COMM_DISCONNECTED;
	}

	EdsError err = EDS_ERR_OK;
	EdsDataType	dataType = kEdsDataType_Unknown;
	EdsUInt32   dataSize = 0;

	if (propertyID == kEdsPropID_Unknown) {
		//If unknown is returned for the property ID , the required property must be retrieved again
		if (err == EDS_ERR_OK) err = getPropertyFromCamera(kEdsPropID_AEModeSelect);
		if (err == EDS_ERR_OK) err = getPropertyFromCamera(kEdsPropID_Tv);
		if (err == EDS_ERR_OK) err = getPropertyFromCamera(kEdsPropID_Av);
		if (err == EDS_ERR_OK) err = getPropertyFromCamera(kEdsPropID_ISOSpeed);
		if (err == EDS_ERR_OK) err = getPropertyFromCamera(kEdsPropID_MeteringMode);
		if (err == EDS_ERR_OK) err = getPropertyFromCamera(kEdsPropID_ExposureCompensation);
		if (err == EDS_ERR_OK) err = getPropertyFromCamera(kEdsPropID_ImageQuality);

		return err;
	}

	//Acquisition of the property size
	if (err == EDS_ERR_OK) {
		err = EdsGetPropertySize(mCamera, propertyID, 0, &dataType, &dataSize);
	}

	if (err == EDS_ERR_OK) {
		if (dataType == kEdsDataType_UInt32) {
			EdsUInt32 data;
			//Acquisition of the property
			err = EdsGetPropertyData(mCamera, propertyID, 0, dataSize, &data);
			//Acquired property value is set
			if (err == EDS_ERR_OK) { setPropertyUInt32(propertyID, data); }
		}

		if (dataType == kEdsDataType_String) {
			EdsChar str[EDS_MAX_NAME];
			//Acquisition of the property
			err = EdsGetPropertyData(mCamera, propertyID, 0, dataSize, str);
			//Acquired property value is set
			if (err == EDS_ERR_OK) { setPropertyString(propertyID, str);}
		}
		if (dataType == kEdsDataType_FocusInfo) {
			EdsFocusInfo focusInfo;
			//Acquisition of the property
			err = EdsGetPropertyData(mCamera, propertyID, 0, dataSize, &focusInfo);
			//Acquired property value is set
			if (err == EDS_ERR_OK){ setFocusInfo(focusInfo);}
		}
	}

	//Update notification
	return err;
}

EdsError CinderCanon::getPropertyDescFromCamera(EdsPropertyID propertyID)
{
	if (!isCameraConnected()) {
		CI_LOG_E(deviceBodyId << " NOT CONNECTED");
		return EDS_ERR_COMM_DISCONNECTED;
	}

	EdsError  err = EDS_ERR_OK;
	EdsPropertyDesc	 propertyDesc = { 0 };

	if (propertyID == kEdsPropID_Unknown) {
		//If unknown is returned for the property ID , the required property must be retrieved again
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

	return err;
}

void CinderCanon::onCameraDisconnected() {
	bCameraIsConnected = false;
	bIsLiveView = false;
	EdsSetObjectEventHandler(mCamera, kEdsObjectEvent_All, NULL, (EdsVoid*)this);
	EdsSetPropertyEventHandler(mCamera, kEdsPropertyEvent_All, NULL, (EdsVoid*)this);
	EdsSetCameraStateEventHandler(mCamera, kEdsStateEvent_All, NULL, (EdsVoid*)this);
	EdsCloseSession(mCamera);
	EdsRelease(mCamera);
	EdsRelease(mCamera_list);
	//deviceIndex = -1;
	signal_disconnected.emit(deviceBodyId);
}


void CinderCanon::shutdown(bool terminateSdk)
{
	console() << "Cinder-Canon :: " << deviceBodyId << " :: shutdown" << endl;
	bCameraIsConnected = false;
	bIsLiveView = false;
	EdsSetObjectEventHandler(mCamera, kEdsObjectEvent_All, NULL, (EdsVoid*)this);
	EdsSetPropertyEventHandler(mCamera, kEdsPropertyEvent_All, NULL, (EdsVoid*)this);
	EdsSetCameraStateEventHandler(mCamera, kEdsStateEvent_All, NULL, (EdsVoid*)this);
	EdsCloseSession( mCamera );
	EdsRelease(mCamera);
	EdsRelease(mCamera_list);
	if (terminateSdk) {
		EdsError err = EDS_ERR_OK;
		err = EdsTerminateSDK();
		if (err == EDS_ERR_OK) {
			CinderCanon::isSDKLoaded = false;
		}
		else {
			console() << "Cinder-Canon :: " << deviceBodyId << " :: error terminating sdk :: " << CanonErrorToString(err) << endl;
		}
	}
}

#pragma mark
#pragma mark Callbacks from the canon SDK
//-----------------------------------------------------------------------------------

void CinderCanon::photoTaken(EdsDirectoryItemRef directoryItem, EdsError error){
	signal_photoTaken.emit(deviceBodyId);
	if (autoDownload) downloadImage(directoryItem); // , this);
}

void CinderCanon::photoDownloaded(const std::string& downloadPath, EdsError error){
	signal_photoDownloaded.emit(deviceBodyId, downloadPath);
}

// Delegate method to tell the canon where to download the full-res image.
std::string CinderCanon::photoDownloadDirectory(){
	return downloadDirectory;
}

