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

