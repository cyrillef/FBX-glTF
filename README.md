<p align="center">
<img src="docs/FBX.png" />
<img src="https://github.com/KhronosGroup/glTF/blob/master/specification/figures/glTF_300.jpg" />
</p>

glTF is the runtime asset format for WebGL, OpenGL ES, and OpenGL.

glTF is a draft specification, it may change before ratification.  Everyone is encouraged to provide feedback on the specification and contribute to the open-source converter.  Please create [issues](https://github.com/KhronosGroup/glTF/issues) with your feedback.

## Specification  

[glTF Specification 0.8](https://github.com/KhronosGroup/glTF/blob/master/specification/README.md)


## Converter

glTF is an open-source command-line pipeline tool that converts FBX file (and any file format that FBX can read such as obj, collada, ...) to glTF.


## FBX importer/exporter plug-in

IO-glTF is an open-source FBX importer/exporter plug-in that converts FBX file (and any file format that FBX can read such as obj, collada, ...) to glTF.<br />
This plug-in can be used by any FBX based application to import/export glTF files.


## Dependencies

This sample is dependent of the following 3rd party extension:

* The [Casablanca C++ RESTful SDK](https://casablanca.codeplex.com/) - version 2.4.0.1

* The [FBX C++ SDK](http://www.autodesk.com/fbx) - version 2015.1

* For building on Mac OSX, and Linux, you also need [cmake](http://www.cmake.org/) which is also required for the Casablanca C++ RESTful SDK.<br />
   However, Casablanca requires version 2.8 (or later), while the glTF converter is setup to use version 3.1, you may install version 3.1 or change the cmake files header back to 2.8.


## Build the solution

### Windows 

Required: Visual Studio 2013 (should work with Visual Studio 2012 too, but not tested)

  1. Download and install the FBX SDK
  
  2. Clone or download the FBX-glTF source code
  
  3. Load the glTF.sln solution file in Visual Studio 2013
  
  4. Optional: if you did not install FBX at the standard location or use a version different from 2015.1, 
       open the solution Property Manager and edit the User Macro FBX_SDK to point to the root of your FBX SDK. 
	   Another way is to edit the Fbx.props file and manually change the path before step 2.

  5. Build the FBX-glTF solution. NuGet will install 'casablanca' for you upon building.
  
  
### Mac OSX

Required: Mac OSX Maverick or Yosemite, Xcode 6.1.1

  1. Download and install the FBX SDK
  
  2. Clone or download the FBX-glTF source code
  
  3. Download and compile 'casablanca' on your machine. [Instructions here](https://casablanca.codeplex.com/wikipage?title=Setup%20and%20Build%20on%20OSX&referringTitle=Documentation)

  4. Install ‘casablanca’ on your system by running ‘sudo make install’
  
  5. Recommended: Create a folder with the name you want, and go in that directory

  6. Optional: Copy the CMakeConfigExample.txt file and rename it as CMakeCache.txt in your build folder,
     and edit the FBX path entry. That will avoid you to type the parameters below.
  
  7. Execute: 
  
     a. For a release build: cmake .. -DCMAKE_BUILD_TYPE=Release -DFBX_SDK=&lt;path to FBX&gt;
	
     b. For a debug build: cmake .. -DCMAKE_BUILD_TYPE=Debug -DFBX_SDK=&lt;path to FBX&gt;

     or simply ‘cmake ..’ if you followed step 6.
     
     
Note: on Mac OSX, the FBX SDK installs by default in /Applications/Autodesk/FBX SDK/2015.1


### Linux

Required: gcc 4.8+ (tested on Ubuntu Linux 14.04 Desktop)

  1. Download and install the FBX SDK
  
  2. Clone or download the FBX-glTF source code
  
  3. Download and compile 'casablanca' on your machine. [Instructions here](https://casablanca.codeplex.com/wikipage?title=Setup%20and%20Build%20on%20Linux&referringTitle=Documentation)
  
  4. Install ‘casablanca’ on your system by running ‘sudo make install’

  5. Recommended: Create a folder with the name you want, and go in that directory
  
  6. Optional: Copy the CMakeConfigExample.txt file and rename it as CMakeCache.txt in your build folder,
     and edit the FBX path entry. That will avoid you to type the parameters below.

  7. Execute: 
  
     a. For a release build: cmake .. -DCMAKE_BUILD_TYPE=Release -DFBX_SDK=&lt;path to FBX&gt;
	
     b. For a debug build: cmake .. -DCMAKE_BUILD_TYPE=Debug -DFBX_SDK=&lt;path to FBX&gt;

     or simply ‘cmake ..’ if you followed step 6.

	
# Usage Instructions

For Windows, the files will be located in  FBX-glTF\x64\Release or FBX-glTF\x64\Debug

For both Mac OXS and Linux, the files will be located in  FBX-glTF/&lt;your cmake folder&gt;/Release or FBX-glTF/<your cmake folder>/Debug

Usage:  glTF [-h] [-v] [-t] [-l] [-e] [-o <output path>] -f &lt;input file&gt;

    -f/--file        - file to convert to glTF [string] 
    -o/--output      - path of output directory [string]
    -n/--name        - override the scene name [string]
    -c/--copy        - copy all media to the target directory (cannot be combined with --embed)
    -e/--embed       - embed all resources as Data URIs (cannot be combined with --copy)
    -h/--help        - this message
    -v/--version     - version

Typical command:

```
glTF -f MyFBXorDAEfile -o MyOutputDirectory -n NewName -c
```

Example:

    cd FBX-glTF\models\duck
    glTF -f duck.fbx -o ..\..\three.js\models\duck -n duck -c
    cd ..\au
    glTF -f au.fbx -o ..\..\three.js\models\au -c
    glTF -f au3.fbx -o ..\..\three.js\models\au3 -e


## Viewing results using Three.js

  1. Install [Node.js](http://nodejs.org/) on your computer
  
  2. Open a node terminal
  
  3. Go in directory FBX-glTF/Three.js
  
  4. Execute: npm install
  
  5. Execute: node serve.js
  
  6. Launch your internet favourite browser and browse to [http://localhost](http://localhost)


--------

## License

This sample is licensed under the terms of the [MIT License](http://opensource.org/licenses/MIT). Please see the [LICENSE](LICENSE) file for full details.


## Written by

Cyrille Fauvel (Autodesk Developer Network)  
http://www.autodesk.com/adn  
http://around-the-corner.typepad.com/  
