<p align="center">
  < width="100" height="100" img src="https://www.napframework.com/png/nap_logo_small.png">
</p>

*	[Description](#description)
	*	[Features](#features)
	*	[Documentation](#documentation)
	*	[Gallery](#gallery)
*	[Compilation](#compilation)
	*	[Dependencies](#dependencies)
	*	[Create the Solution](#create-the-solution)
	*	[Run a Demo](#run-a-demo)
	*	[Work Against Source](#compile-your-project-against-nap-source)
	*	[Package](#build-your-own-nap-distribution-package)
	
# Description

[NAP framework](https://www.napframework.com) is an open source, data-driven platform that merges game technology with the flexibility of a creative coding environment. NAP allows you to create fast, modular and (above all) stable applications. 

Use any protocol (OSC, MIDI, Art-Net, Websockets etc) in combination with a 3D graphics and sound engine to create real-time content that is transmitted to any device you like. NAP is built to scale up to a large number of input and outputdevices: many speakers, many lights, many screens, many sensors.
	
NAPs design is intended to be as open as possible: there is little fixed functionality, but there are a lot of useful blocks that can be tied together by a user to create the experience you desire. As a user, you can build new blocks yourself and throw them in the mix to fulfill your creative vision. To push creativity, NAP is built to provide extremely fast iteration times.

Central to NAP are a couple of key philosophies:

- NAP is completely data driven
- NAP is heavily influenced by modern game engine design, with one exception:
- NAP does not dictate any sort of pipeline, you decide how you render, couple devices and create sound etc.
- NAP applications are lean and mean, only package and ship what you actually use
- NAP is easy to extend: build your own modules, resources, devices and components
- NAP wants you to be safe and validates data for you
- NAP is responsive: hot-load content changes directly in to the running app
- NAP is completely cross-platform and supports all modern desktop environments
- NAP app content can be edited using an editor

## Features

NAP Framework ships with many useful modules, including: a Vulkan 2D/3D render engine, an audio engine for music playback, recording and analysis, a data sequencer, an editor to author application content, a system for creating and loading presets, a video player powered by FFMPEG and a Python programming interface.

NAP also has built in support for many common protocols and standards, including: Websockets, Midi, OSC, Artnet, Serial, Ethercat, OpenCV and SQLite. NAP has been battle tested in production for years. For more information about NAP, how it is commonly used and what it can do for you, visit the [napframework](https://www.napframework.com) website.

<p align="center">
  <img src="https://www.napframework.com/png/Vulkan_170px_Dec16.png">
</p>

## Documentation

NAP documentation can be found online at [www.napframework.com/doxygen](https://www.napframework.com/doxygen/). Take note that the installation and project creation instructions on that website apply to the precompiled NAP Package only. Follow the instructions in this document to build and create a NAP application from source. All other parts of the documentation apply to both the NAP package and source context.

## Gallery

Visit [www.napframework.com](https://www.napframework.com/showcase) for more examples

![Between Mind and Matter, Nick Verstand](https://www.napframework.com/jpg/bmm_1280.jpg)
[Between Mind and Matter](http://www.nickverstand.com/) by Nick Verstand, Marcel Smit and 4DSOUND
![Habitat, Heleen Blanken](https://www.napframework.com/jpg/habitat_1280.jpg)
[Habitat](https://www.heleenblanken.com/habitatbyheleenblanken) by Heleen Blanken, Naivi and Stijn van Beek
![EGO, Studio Drift](https://www.napframework.com/jpg/ego_1280.jpg)
[EGO](https://www.studiodrift.com/work#/ego/) by Studio Drift
![4DSound System](https://www.napframework.com/jpg/4d-sound-full.jpg)
[4DSound System](https://4dsound.net/)

# Compilation

## Dependencies

To generate a solution and compile the source code you need to have installed: 

- [Qt 5](http://download.qt.io/official_releases/qt/)
	- The precompiled package uses QT 5.11.3, although other versions are known to work.
	- Use the QT Online Installer and select the **Archive** package category to access older versions
- Latest version of [Git](https://git-scm.com/download/win)

NAP depends on various other third party libraries. A set of compatible libraries can be downloaded from our github page. Put the thirdparty library directory next to the NAP source directory:

- ~/dev
	- nap
	- thirdparty

NAP requires that your Qt version is a build from [qt.io](http://download.qt.io/official_releases/qt/) and that the environment variable `QT_DIR` points to the directory that holds the libraries, e.g.: `C:\mycomp\qt\5.11.3\msvc2015_64`.

## Create the Solution

Run:

`check_build_environment` to ensure your build environment is up to date.

On success, run:

`generate_solution.sh` to generate an `XCode project` (OSX)<br>
`generate_solution.bat` to generate a `Visual Studio Solution` (Win64)<br>
`generate_solution.sh` to generate `make files` (Linux)<br>

The solution allows you to build every target and inspect the code of the demos, editor, modules, core etc. NAP uses a pre-bundled version of CMake in third-party to ensure compatibility for all platforms. The default build configuration is `debug`. Alternatively you can use `CLion`.

## Run a Demo

Open the generated solution in `XCode` or `Visual Studio`, select a build configuration (`Debug`or `Release`) and a demo as target. Compile and run the demo. You can also use the `build` script to compile one or more projects using the command line, for example: `sh build.sh target:helloworld`.

---

## Compile your project against NAP source
### Motivation
Allows you to step into the NAP Framework source code and make changes if required.

### Process
* To see how you set up an app in source, look at the example in the `apps` folder.
* Add your project to the main `CMakeLists.txt` file

 Running `./generate_solution.sh` or `./generate_solution.bat` will re-create the Xcode or Visual Studio solution and include your project.

---

## Build your own NAP distribution package
A packaged version of NAP will include all of the following:
* core components
* standard modules
* demos
* your own project(s) - only if specified

After packaging a new zip or folder is created, with the naming convention `NAP`-*Version*-*Platform*-*Timestamp* (Timestamp may be optionally omitted).

**By default only headers and binaries are included; source code and debug symbols will be excluded.**

## Package for Desktop OS

To package NAP for release run: `package.bat` (Windows) or `package.sh` (MacOS / Linux). You can prefix the environment variable for the location of your Qt Framework library if necessary, e.g.
```
QT_DIR=/home/myusername/Qt/5.11.3/gcc_64 ./package.sh
```

This will compile a package including all demos but without your own projects (defined in source). Alternatively, you can use the `-sna` flag to build a package including your own project (plus demos), e.g.:
```
./package.sh -sna MyProject
```

Some other useful flags:
* `-nt`: remove the timestamp
* `-nz`: do not create a zip file from the release
* `-ds`: include debug symbols. On windows .pdb files are also packaged.

More options for packaging can be queried by adding the flag `--help` when running the script.

## Package for Android

At the moment Android is not supported because of recent changes to the build system.
You can attempt to package NAP for Android but it will most likely fail. To package NAP for Android you need to have installed:

- The Android NDK 
- [Ninja](https://github.com/ninja-build/ninja/releases)
	- Added to system path
- [Git](https://git-scm.com/download)
	- Added to system path

To package NAP for Android run: `package.bat` or `package.sh` together with the `--android` flag. To use the package in production combine the `--android` flag with `-nt` (no timestamp) and `-nz` (no zip). You must point the `ANDROID_NDK_ROOT` environment variable to the install location of the Android NDK.
