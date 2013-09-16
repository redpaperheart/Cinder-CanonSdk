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