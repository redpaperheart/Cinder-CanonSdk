#include "cinder/app/AppNative.h"
#include "cinder/Utilities.h"
#include "cinder/gl/gl.h"
#include "cinder/gl/Texture.h"
#include "cinder/Timeline.h"

#include "CinderCanon.h"
#include "CanonCamera.h"

using namespace ci;

// Subclass from Photohandler to handle full-res photos once they are downloaded
class _TBOX_PREFIX_App : public AppNative, public canon::PhotoHandler
{
public:
	void prepareSettings( Settings *settings ) {};
	void setup();
	void update();
	void draw();
    void shutdown();
    
    void keyDown( KeyEvent event );
    void takePhoto();
    
    // Delegate callbacks from canon::PhotoHandler
    void photoTaken(EdsDirectoryItemRef directoryItem, EdsError error);
    void photoDownloaded(const std::string & downloadPath, EdsError error);
    std::string photoDownloadDirectory();
    
    CanonCamera                 mCamera;
    fs::path                    mPhotoDownloadFolder;
    Anim<float>                 mFlashAlpha;
};

void _TBOX_PREFIX_App::setup()
{
    mFlashAlpha = 0.0f;
    mPhotoDownloadFolder = getHomeDirectory()/"Documents"/"Canon Pictures";
    if (!fs::exists(mPhotoDownloadFolder)) fs::create_directories(mPhotoDownloadFolder);
    
    mCamera.setup();
    mCamera.startLiveView();}



void _TBOX_PREFIX_App::update()
{
    mCamera.update();
    if(mCamera.checkNewFrame()){
        // process image here with mCamera.getLiveSurface();
    }
}

void _TBOX_PREFIX_App::draw()
{
    gl::enableAlphaBlending();
    gl::clear(Color::black());
    if(mCamera.isLiveViewing()) mCamera.draw(getWindowBounds());
    
    if (mFlashAlpha > 0.0f) {
        gl::color(1.0f, 1.0f, 1.0f, mFlashAlpha);
        gl::drawSolidRect(getWindowBounds());
    }
}

void _TBOX_PREFIX_App::takePhoto()
{
    mCamera.endLiveView();
    mCamera.takePicture(this);
    mFlashAlpha = 1.0f;
    timeline().apply(&mFlashAlpha, 0.0f, 3.0f);
}

void _TBOX_PREFIX_App::keyDown( KeyEvent event )
{
    switch (event.getChar())
    {
        case 'l': mCamera.toggleLiveView(); break;
        case 'f': setFullScreen( !isFullScreen() ); break;
        case ' ': takePhoto(); break;
        case event.KEY_ESCAPE: quit(); break;
    }
}

void _TBOX_PREFIX_App::shutdown()
{
    mCamera.shutdown();
}

#pragma mark
#pragma mark Callbacks from the canon SDK
//-----------------------------------------------------------------------------------

void _TBOX_PREFIX_App::photoTaken(EdsDirectoryItemRef directoryItem, EdsError error)
{
    mCamera.downloadImage(directoryItem, this);
}

void _TBOX_PREFIX_App::photoDownloaded(const std::string & downloadPath, EdsError error)
{
    mCamera.startLiveView();
}

// Delegate method to tell the canon where to download the full-res image.
std::string _TBOX_PREFIX_App::photoDownloadDirectory()
{
    return mPhotoDownloadFolder.generic_string();
}


CINDER_APP_NATIVE( _TBOX_PREFIX_App, RendererGl )
