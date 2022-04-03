<br>
<p align="center">
  <img width=384 src="https://docs.nap.tech/content/nap_logo_blue.svg">
</p>

*	[Description](#description)
	*	[Features](#features)
	* [Philosophy](#philosophy)
	*	[Documentation](#documentation)
	*	[Gallery](#gallery)
*	[Where to Start](#where-to-start)
	* [System Compatibility](#system-compatibility)
	* [Binary Packages](#binary-packages)
	* [Raspberry Pi](#raspberry-pi)
*	[Compilation](#compilation)
	*	[Dependencies](#dependencies)
	*	[Create the Solution](#create-the-solution)
	*	[Run a Demo](#run-a-demo)
	*	[Work Against Source](#compile-your-project-against-nap-source)
	*	[Package](#build-your-own-nap-distribution-package)
*	[Contributing](#contributing)
*	[License](#license)
	
# Description

[NAP](https://nap.tech) is an open source, low overhead, real-time control & visualization plaform. Create fast, modular and (above all) stable applications to interact with the world around you.

Use any protocol (OSC, MIDI, Artnet, WebSocket etc) in combination with a 3D graphics and sound engine to create real-time content that is transmitted to any device you like. NAP is built to scale up to a large number of input and output devices: many displays, many lights, many speakers, many sensors, many servos.

## Features

NAP Framework ships with many useful modules, including: a Vulkan 2D/3D render engine, a Vulkan Compute module, a multi-channel audio engine for music playback, recording and analysis, a sequencer to control parameters in real-time, an editor to author application content, a web-portal to control and monitor NAP applications in a browser, a system for creating and loading presets, a video player powered by FFmpeg and a Python programming interface.

NAP also has built in support for many common protocols and standards, including: WebSocket, MIDI, OSC, Artnet, Serial, EtherCAT, OpenCV and SQLite. NAP has been battle tested in production for years. For more information about NAP, how it is commonly used and what it can do for you, visit the [nap.tech](https://nap.tech) website.

<br>
<p align="center">
  <img width=256 img src="https://docs.nap.tech/content/vulkan-logo.svg" >
</p>

## Philosophy

NAP is completely data driven and heavily influenced by modern game engine design, with one exception: it does not dictate any sort of pipeline. This allows NAP to run on any type of device: from low-power, energy efficient ARM computers such as the Raspberry Pi to industrial PCs and x86 gaming rigs. 

NAP applications are lean and mean: only package and ship what you actually use. On top of that NAP is easy to extend: build you own modules, resources, devices and components. NAP wants you to be safe and validates data for you on initialization. Applications are also responsive: hot-load content changes directly in to the running application. On top of that NAP is completely cross-platform and supports all modern desktop environments.

## Documentation

NAP documentation can be found online at [docs.nap.tech](https://docs.nap.tech/pages.html). Take note that the installation and project creation instructions on that website apply to the binary (compiled) NAP package only. Follow the instructions in this document to build and work against NAP Framework from source.

## Gallery

Visit [nap-labs.tech](https://nap-labs.tech) for more examples

![Between Mind and Matter, Nick Verstand](https://docs.nap.tech/additional_content/bmm_1280.jpg)
[Between Mind and Matter](http://www.nickverstand.com/) by Nick Verstand, Marcel Smit and 4DSOUND
![Habitat, Heleen Blanken](https://docs.nap.tech/additional_content/habitat_1280.jpg)
[Habitat](https://www.heleenblanken.com/habitatbyheleenblanken) by Heleen Blanken, Naivi and Stijn van Beek
![Shylight, Studio Drift](https://docs.nap.tech/additional_content/shylight_basel_1280.jpg)
[Shylight](https://www.studiodrift.com/work#/work/shylight/) by Studio Drift
![4DSound System](https://docs.nap.tech/additional_content/4D_1280.jpg)
[4DSound System](https://4dsound.net/)
![NAP Framework](https://docs.nap.tech/additional_content/napkin_1280.jpg)
[NAP Framework](https://nap.tech) editor & demo

# Where to Start

## System Compatibility

Currently, whether working with the packaged framework release or against the framework source, we support the following architectures and operating systems:

**x86**
```
x86-64: Windows 10, Visual Studio 2019 (v142)
x86-64: Ubuntu Linux LTS (v20.04)
x86-64: macOS Catalina (v10.15)
```
**ARM**
```
armhf: Raspberry Pi OS (v11)
arm64: Ubuntu Linux LTS (v20.04) *experimental*
```

## Binary Packages

NAP's official releases are provided as binary packages at [www.napframework.com](https://nap.tech) and [github](https://github.com/napframework/nap/releases). For most developers this is the best place to start. Our developer experience is tuned to work with the releases there, where the process of managing projects and modules is streamlined. Certain functionality, eg. packaging a project for distribution, is also only available through a packaged framework release, which is created from source. 

When working against a binary package, follow the official [installation](https://docs.nap.tech) instructions, instead of the instructions in this document. Continue reading below to compile and get started with the NAP source code.

## Raspberry Pi

Only the `Raspberry Pi 4` running `Debian Bullseye (v11, armhf)` is 'fully' supported. Targets that make use of the Vulkan render module 'might' run on older Raspberry Pi models, but without hardware acceleration, using the `LLVMpipe` instead of the integrated `Mali GPU`. Headless applications and services without graphics should run on older models, although this has not been tested. The editor (napkin) only works on the Raspberry Pi 4.

# Compilation

Windows 10<br/>
[![Build Status](http://build.nap-labs.tech:8092/app/rest/builds/buildType:(id:Public_PackageNapWin64)/statusIcon)](http://build.nap-labs.tech:8092/viewType.html?buildTypeId=myID&guest=1)

Ubuntu 20.04<br/>
[![Build Status](http://build.nap-labs.tech:8092/app/rest/builds/buildType:(id:Public_PackageNapLinux)/statusIcon)](http://build.nap-labs.tech:8092/viewType.html?buildTypeId=myID&guest=1)

macOS 10.15<br/>
[![Build Status](http://build.nap-labs.tech:8092/app/rest/builds/buildType:(id:Public_PackageNapOsx)/statusIcon)](http://build.nap-labs.tech:8092/viewType.html?buildTypeId=myID&guest=1)

Raspbian 11<br/>
[![Build Status](http://build.nap-labs.tech:8092/app/rest/builds/buildType:(id:Public_PackageNapRaspberryPi)/statusIcon)](http://build.nap-labs.tech:8092/viewType.html?buildTypeId=myID&guest=1)

## Dependencies

To generate a solution and compile the source code you need to have installed: 

- Qt 5
	- x86_64
		- The precompiled package uses Qt 5.12 (LTS), although other versions are known to work.
		- Go to [qt.io](https://www.qt.io/download) and select **Downloads for open source users**
		- Download the Qt online installer
		- During installation select **Custom installation** 
		- Filter on the **LTS** category to download and install Qt 5.12 for your target platform
	- ARM
		- [Download](https://docs.nap.tech/additional_content/qt-5.15.2-armhf-pi4-raspbian_bullseye.tar.xz) Qt 5.15.2 for Raspberry Pi OS 11 *armhf*
		- [Download](https://docs.nap.tech/additional_content/qt-5.15.2-arm64-ubuntu_20.04.tar.xz) Qt 5.15.2 for Ubuntu 20.04 *arm64*

NAP also depends on a small set of **precompiled** third party libraries. The precompiled libraries can be [downloaded](https://github.com/napframework/thirdparty) from our Github page. Put the thirdparty directory next to the NAP source directory:

- /dev
	- nap
	- thirdparty

Create an environment variable called `QT_DIR` and point it to the directory that contains the QT libraries, for example: `C:\qt\5.12.11\msvc2015_64`. The build system uses this environment variable to locate QT. Note that only the editor (Napkin) depends on Qt, NAP applications do not have a dependency on Qt.

On Windows, make sure the [Visual C++ 2013 Redistributable (x64)](https://www.microsoft.com/en-us/download/details.aspx?id=40784) is installed. This is (unfortunately) required because of a third party dependency. 

## Create the Solution

Run:

`check_build_environment` to ensure your build environment is up to date.

On success, run:

`generate_solution.sh` to generate an `XCode project` (macOS)<br>
`generate_solution.bat` to generate a `Visual Studio Solution` (Windows)<br>
`generate_solution.sh` to generate `make files` (Linux)<br>

The solution allows you to build every target and inspect the code of the demos, editor, modules, core etc. NAP uses a pre-bundled version of CMake in third-party to ensure compatibility for all platforms. The default build configuration is `debug`. Alternatively you can use `CLion`.

## Run a Demo

Open the generated solution in `XCode` or `Visual Studio`, select a build configuration (`Debug`or `Release`) and a demo as target. Compile and run the demo. You can also use the `build` script to compile one or more projects using the command line, for example: `sh build.sh target:helloworld`.

---

## Compile your project against NAP source
### Motivation
Allows you to step into the NAP Framework source code and make changes if required. If access to the NAP source code is not required during development it is advised to work against a pre-compiled NAP package instead.

### Process
* To see how you set up an app in source, look at the example in the `apps` folder.
* Add your project to the main `CMakeLists.txt` file

 Running `./generate_solution.sh` or `generate_solution.bat` will re-create the Xcode project, make files or Visual Studio solution and include your project.

---

## Build your own NAP distribution package
A packaged version of NAP will include all of the following:
* core components
* standard modules
* editor
* demos
* your own project(s) - only if specified

After packaging a new zip or folder is created, with the naming convention `NAP-*Version*-*Platform*-*Timestamp*` (Timestamp may be optionally omitted).

**By default only headers and binaries are included; source code and debug symbols will be excluded.**

## Package

To package NAP run: `package.bat` (Windows) or `package.sh` (macOS / Linux). You can prefix the environment variable for the location of your Qt Framework library if necessary, e.g.
```
./package.sh
```

This will compile a package including all demos but without your own projects (defined in source). Alternatively, you can use the `-sna` flag to build a package including your own project (plus demos), e.g.:
```
./package.sh -sna MyProject
```

Some other useful flags:
* `-nt`: remove the timestamp
* `-nz`: do not create a zip file from the release
* `-ds`: include debug symbols. On Windows .pdb files are also packaged.

More options for packaging can be queried by adding the flag `--help` when running the script.

# Contributing

We welcome contributions and potential bug fixes. But before you submit any code for review make sure to read and follow our [C++ styleguide](styleguide/styleguide.md). Also take into consideration that reviewing code takes time: Be as thorough and explicit as possible. 

Do not use the github `issues` page to ask questions. We already have a perfectly well functioning [forum](https://community.napframework.com/) for that. Only use the github `issues` page for bug reports and well defined feature requests. 

New modules are not considered unless useful, vital or important enough to have as part of the core release. If you feel a module is missing we would like to [hear](https://community.napframework.com/) from you. If a module depends on a third-party library, linkage should be dynamic and not violate the NAP license policy. Static linkage is discouraged unless recommended by the library or when a NAP application, that uses the module, doesn't require the library to link and run. In that case all third-party code is compiled into the module when NAP is packaged. Third-party dependencies must work cross-platform and must be compiled using
```
MSVC, Platform Toolset v142 on Windows 10
Clang targeting Catalina/10.15 on macOS
GCC <= 9.3.0 on Ubuntu LTS 20.04
```

# License

NAP Framework is open source software, licensed under the [Mozilla Public License Version 2.0](https://www.mozilla.org/en-US/MPL/2.0/). You can use the NAP source code and NAP distributable package to create both commercial and non-commercial derivate works, as long as the MPL 2.0 license is not violated. 

This means that you must make your changes to the NAP source code available under MPL. You can combine NAP source code with proprietary code, as long as you keep the NAP source code in separate files. Version 2.0 is, by default, compatible with LGPL and GPL version 2 or greater. You can also distribute binaries under a proprietary license, as long as you make the NAP source code available under MPL. 

In short: You are allowed to use and modify the NAP source code, for both commercial and non commercial purposes. However: if you make changes to the NAP source code, you have to make those changes available for others to use. We encourage people to share their ideas with the community. Keeping knowledge to yourself is not what NAP Framework is about.
