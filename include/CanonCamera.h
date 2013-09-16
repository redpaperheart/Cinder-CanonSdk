#pragma once

#include "cinder/app/AppBasic.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"

#include "CinderCanon.h"

using namespace ci;
using namespace ci::app;
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
