# houdini-sop-color-lut

[Houdini](http://www.sidefx.com/index.php) SOP node which loads the LUT color data and applies it to geometry.

![houdini-sop-color-lut screenshot](http://i.imgur.com/h4A6u2c.png)

## Binaries, Houdini 15
* [Mac OS X, Houdini 15.0.434](https://github.com/ttvd/houdini-sop-color-lut/releases/download/1.0/houdini.sop.color.lut.15.0.434.osx.tar.gz)
* [Windows, Houdini 15.0.440](https://github.com/ttvd/houdini-sop-color-lut/releases/download/1.0/houdini.sop.color.lut.15.0.440.win.rar)

## Building

* Tested on OS X 10.11 / Windows and Houdini 15.
  * You would have to patch CMake file to get this building on Linux.
* Define HOUDINI_VERSION env variable to be the version of Houdini 15 you wish to build against (for example "15.0.313").
* Alternatively, you can have HFS env variable defined (set when you source houdini_setup).
* Generate build files from CMake for your favorite build system. For Windows builds use MSVC 2012.
* Build the SOP Houdini dso (SOP_ColorLUT.dylib or SOP_ColorLUT.dll).
* Place the dso in the appropriate Houdini dso folder.
  * On OS X this would be /Users/your_username/Library/Preferences/houdini/15.0/dso/
  * On Windows this would be C:\Users\your_username\Documents\houdini15.0\dso

## Supported LUT formats

* [MagicaVoxel](https://voxel.codeplex.com/) .vox 256 color palette (with fallback to default .vox palette).
* .png files (either 1d (left to right) or 2d (left/top sequential order)).

## Usage

* Place Color LUT SOP in your SOP network.
* If enabled, the default 256 color palette will be used (default [MagicaVoxel](https://voxel.codeplex.com/) palette).
* Optionally, specify file path to a supported LUT file (please see the list of supported formats).
* Specify class of attribute (point, vertex, primitive or detail).
* Specify sampling behavior (clamp or wrap).
* Specify LUT input attribute name. These values will be used to index into LUT. This can be either an integer or a float attribute.
* Specify to optionally remove the input LUT attribute.

## Future work

* Add support for [Houdini](http://www.sidefx.com/index.php) .lut and .blut formats.

## License

* Copyright Mykola Konyk, 2016
* Distributed under the [MS-RL License.](http://opensource.org/licenses/MS-RL)
* **To further explain the license:**
  * **You cannot re-license any files in this project.**
  * **That is, they must remain under the [MS-RL license.](http://opensource.org/licenses/MS-RL)**
  * **Any other files you add to this project can be under any license you want.**
  * **You cannot use any of this code in a GPL project.**
  * Otherwise you are free to do pretty much anything you want with this code.
