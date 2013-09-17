Cinder-CanonSdk
===============

Cinder wrapper for the Canon Digital EOS SDK v2.13. Tested only on Mac OS X.
Created at Red Paper Heart by Charlie Whitney, Jamie Kosoy and Adrià Navarro.      
Based on ofxCanon by Diederick Huijbers and ofxEdsdsk by Kyle Mcdonald.

Installation
------------
1. Clone the block into your addons folder
2. Register as Canon developer and download the SDK from any of the following sites:
    - [Canon USA](http://consumer.usa.canon.com/cusa/support/consumer/eos_slr_camera_systems/eos_digital_slr_cameras/digital_rebel_xt?fileURL=ps_sdk_form&pageKeyCode=downloadLicense&id=0901e02480057a74_1&productOverviewCid=0901e0248003ce28&keycode=Sdk_Lic)
    - [Canon Europa](https://www.didp.canon-europa.com/)
3. Unzip the file, open the .dmg and copy the folder called EDSDK into the *lib/* folder of the addon.
4. Create a new project with Tinderbox, either starting from scratch or using the template.
5. TO FIX: if you get a *Library not loaded* error when starting the app, go to the Copy Files section of Build Phases and select *Frameworks* from the Destionation dropdown menu. Trying to figure out how to include this in the tinderbox config xml.

Usage
------
For basic usage, create a *CanonCamera* object, call setup() at the beginning and start and stop liveView as you want. You'll need to update it every frame. With *getLiveSurface()* you can process the image and draw it in a more flexible way than doing *camera.draw()*.

To get a callback when a photo has been taken your app will need to subclass *canon::PhotoHandler* and implement three delegate methods: *photoTaken(), photoDownloaded()* and *photoDirectory()*, as shown in the example.

For advanced usage you can get an instance of the *CinderCanon* class from the CanonCamera, and send specific commands to the device. There's also an experimental recording mode that hasn't been tested, and some placeholders for upcoming functionality.

License
-------
Copyright (c) 2012, Red Paper Heart, All rights reserved. To contact Red Paper Heart, email hello@redpaperheart.com or tweet @redpaperhearts

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
 
Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
 
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.