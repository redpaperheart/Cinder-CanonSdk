Cinder-CanonSdk
===============

Cinder wrapper for the Canon Digital EOS SDK v2.13. Tested only on Mac OS X.

Installation
------------
1. Clone the block into your addons folder
2. Register as Canon developer and download the SDK from any of the following sites:
1.1. [Canon USA](http://consumer.usa.canon.com/cusa/support/consumer/eos_slr_camera_systems/eos_digital_slr_cameras/digital_rebel_xt?fileURL=ps_sdk_form&pageKeyCode=downloadLicense&id=0901e02480057a74_1&productOverviewCid=0901e0248003ce28&keycode=Sdk_Lic)
1.2. [Canon Europa](https://www.didp.canon-europa.com/)
3. Unzip the file, open the .dmg and copy the folder called EDSDK into the lib/ folder of the addon.
4. Create a new project with Tinderbox, either starting from scratch or using the template.
5. TO FIX: if you get a Library not loaded error when starting the app, go to the Copy Files section of Build Phases and select **Frameworks** from the Destionation dropdown menu. Trying to figure out how to include this in the tinderbox config xml.

