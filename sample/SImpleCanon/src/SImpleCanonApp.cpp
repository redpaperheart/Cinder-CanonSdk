#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Log.h"
#include "cinder/Timeline.h"
#include "cinder/Utilities.h"

//#include "CinderCanon.h"
#include "CanonCamera.h"

//canon
#include "EDSDK.h"
#include "EDSDKErrors.h"
#include "EDSDKTypes.h"

using namespace ci;

// Subclass from Photohandler to handle full-res photos once they are downloaded
class SimpleCanonApp : public app::App
{
public:
	void setup();
	void update();
	void draw();
    void cleanup();

    void keyDown( ci::app::KeyEvent event );
    void takePhoto();

    // Delegate callbacks from canon::PhotoHandler
    void photoTaken(EdsDirectoryItemRef directoryItem, EdsError error);
    void photoDownloaded(const std::string & downloadPath, EdsError error);
    std::string getPhotoDownloadDirectoryString();
    static EdsError EDSCALLBACK handleCameraAdded(EdsVoid* inContext);
    
    //signals from cameras
    void onSignalPhotoTaken(std::string deviceBodyId);
    void onSignalPhotoDownloaded(std::string deviceBodyId, std::string photoPath);
    void onSignalDisconnected(std::string deviceBodyId);
    
    ci::canon::CanonCamera      mCamera;
    fs::path                    mPhotoDownloadFolder;
    Anim<float>                 mFlashAlpha;
    gl::TextureRef              mTextureRef;
};

void prepareSettings(ci::app::App::Settings* settings)
{
    settings->setWindowSize(960, 640);
//    settings->setMultiTouchEnabled(false);
}

void SimpleCanonApp::setup()
{
    CI_LOG_I("setup");
    mTextureRef = gl::Texture::create(928, 616);
    mFlashAlpha = 0.0f;
    mPhotoDownloadFolder = getHomeDirectory()/"Documents"/"Canon Pictures";
    if (!fs::exists(mPhotoDownloadFolder)) fs::create_directories(mPhotoDownloadFolder);
    
    
    mCamera.setup();
    mCamera.signal_disconnected.connect(std::bind(&SimpleCanonApp::onSignalDisconnected, this, std::placeholders::_1));
    mCamera.signal_photoTaken.connect(std::bind(&SimpleCanonApp::onSignalPhotoTaken, this, std::placeholders::_1));
    mCamera.signal_photoDownloaded.connect(std::bind(&SimpleCanonApp::onSignalPhotoDownloaded, this, std::placeholders::_1, std::placeholders::_2));
                
    mCamera.startLiveView();
    mCamera.downloadDirectory = getPhotoDownloadDirectoryString();
}

void SimpleCanonApp::update()
{
    if (mCamera.isLiveViewing()) {
        mCamera.downloadData();
    }
}

void SimpleCanonApp::draw()
{
    gl::enableAlphaBlending();
    gl::clear(Color::white());

    if (mCamera.isLiveViewing()) {
        gl::color(1.0f, 1.0f, 1.0f, 1.0f);
        Surface8uRef sf = mCamera.getLiveSurface();
        if (sf) {
            mTextureRef->update(*sf);
            gl::draw(mTextureRef);
        }
    }

    if (mFlashAlpha > 0.0f) {
        gl::color(1.0f, 1.0f, 1.0f, mFlashAlpha);
        gl::drawSolidRect(ci::app::getWindowBounds());
    }
}

void SimpleCanonApp::takePhoto()
{
//    mCamera.endLiveView();
//    mCamera.takePicture();
    mFlashAlpha = 1.0f;
    app::timeline().apply(&mFlashAlpha, 0.0f, 3.0f);
}

void SimpleCanonApp::keyDown( ci::app::KeyEvent event )
{
    switch (event.getChar())
    {
//        case 'l': mCamera.toggleLiveView(); break;
        case 'f': ci::app::setFullScreen( !ci::app::isFullScreen() ); break;
        case ' ': takePhoto(); break;
//        case event.KEY_ESCAPE: quit(); break;
    }
}

void SimpleCanonApp::onSignalDisconnected(std::string deviceBodyId) {
    CI_LOG_W(deviceBodyId << " :: onSignalDisconnected.");
}

void SimpleCanonApp::onSignalPhotoTaken(std::string deviceBodyId) {
    CI_LOG_D(deviceBodyId  << " :: onSignalPhotoTaken.");
}
void SimpleCanonApp::onSignalPhotoDownloaded(std::string deviceBodyId, std::string photoPath) {
    CI_LOG_I(deviceBodyId << " :: onSignalPhotoDownloaded :: " << photoPath);
    mCamera.startLiveView();
}

void SimpleCanonApp::cleanup()
{
    CI_LOG_I("cleanup");
    mCamera.cleanup();
}

#pragma mark
#pragma mark Callbacks from the canon SDK
//-----------------------------------------------------------------------------------

EdsError EDSCALLBACK SimpleCanonApp::handleCameraAdded(EdsVoid* inContext) {
//    SimpleCanonApp* cm = (SimpleCanonApp*)inContext;
    CI_LOG_I("Camera added Event.");
        
//        if (Model::getInstance()->getCurState() == Model::STATE_EXPERIENCE_ACTIVE && cm->getNumConnectedDevices() > 0) {
//            cm->signal_log.emit("info", "[PHOTOS] CanonManager :: New Camera dected. Waiting until experience is in Idle before reloading devices.", "");
//            cm->mReloadCameras = true;
//        }
//        else {
//            cm->signal_log.emit("info", "[PHOTOS] CanonManager :: New Camera dected. Shutting down cameras to add all available devices.", "");
//            cm->closeAllDevices();
//            cm->openAvailableDevices();
//        }
    return EDS_ERR_OK;
}

void SimpleCanonApp::photoTaken(EdsDirectoryItemRef directoryItem, EdsError error)
{
//    mCamera.downloadImage(directoryItem, this);
}

void SimpleCanonApp::photoDownloaded(const std::string & downloadPath, EdsError error)
{
//    mCamera.startLiveView();
}

// Delegate method to tell the canon where to download the full-res image.
std::string SimpleCanonApp::getPhotoDownloadDirectoryString()
{
    return mPhotoDownloadFolder.generic_string();
}


CINDER_APP(SimpleCanonApp, ci::app::RendererGl, prepareSettings)
