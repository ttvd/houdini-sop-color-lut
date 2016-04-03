# houdini-sop-color-lut

[Houdini](http://www.sidefx.com/index.php) SOP node which loads the LUT color data and applies it to geometry.

![houdini-sop-color-lut screenshot](http://i.imgur.com/Ao2zSxy.png)

## Binaries, Houdini 15
* Mac OS X - Todo.
* Windows - Todo.

## Building

* Tested on OS X 10.11 and Houdini 15.
  * You would have to patch CMake file to get this building on Windows or Linux.
* Define HOUDINI_VERSION env variable to be the version of Houdini 15 you wish to build against (for example "15.0.313").
* Alternatively, you can have HFS env variable defined (set when you source houdini_setup).
* Generate build files from CMake for your favorite build system.
* Build the SOP Houdini dso (SOP_ColorLUT.dylib or SOP_ColorLUT.dll).
* Place the dso in the appropriate Houdini dso folder.
  * On OS X this would be /Users/your_username/Library/Preferences/houdini/15.0/dso/

## Supported LUT formats

* [MagicaVoxel](https://voxel.codeplex.com/) .vox (with fallback to default .vox palette).
* [Houdini](http://www.sidefx.com/index.php) .lut and .blut formats.

## Usage

* Place Color LUT SOP in your SOP network.
* If enabled, the default 256 color palette will be used (default [MagicaVoxel](https://voxel.codeplex.com/) palette).
* Optionally, specify file path to a supported LUT file (please see the list of supported formats).
* Specify class of attribute (point, vertex, primitive or detail).
* Specify LUT input attribute name. These values will be used to index into LUT. This can be either an integer or a float attribute.
* Specify to optionally remove the input LUT attribute.

## License

* Copyright Mykola Konyk, 2016
* Distributed under the [MS-RL License.](http://opensource.org/licenses/MS-RL)
* **To further explain the license:**
  * **You cannot re-license any files in this project.**
  * **That is, they must remain under the [MS-RL license.](http://opensource.org/licenses/MS-RL)**
  * **Any other files you add to this project can be under any license you want.**
  * **You cannot use any of this code in a GPL project.**
  * Otherwise you are free to do pretty much anything you want with this code.
