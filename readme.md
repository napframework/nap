<p align="center">
  <img width=256 height=256 src="https://www.napframework.com/png/nap_logo_small.png">
</p>

*	[Description](#description)
	*	[Features](#features)
	*	[Documentation](#documentation)
	*	[Gallery](#gallery)
*	[Where to Start](#where-to-start)
*	[Compilation](#compilation)
	*	[Dependencies](#dependencies)
	*	[Create the Solution](#create-the-solution)
	*	[Run a Demo](#run-a-demo)
	*	[Work Against Source](#compile-your-project-against-nap-source)
	*	[Package](#build-your-own-nap-distribution-package)
*	[Contributing](#contributing)
*	[License](#license)
	
# Description

[NAP framework](https://www.napframework.com) is an open source, low overhead, real-time control & visualization plaform. Create fast, modular and (above all) stable applications to interact with the world around you.

Use any protocol (OSC, MIDI, Artnet, WebSocket etc) in combination with a 3D graphics and sound engine to create real-time content that is transmitted to any device you like. NAP is built to scale up to a large number of input and output devices: many displays, many lights, many speakers, many sensors, many servos.

NAP's design is intended to be as open as possible: there is little fixed functionality, but there are a lot of useful blocks that can be tied together by a user to create the experience you desire. As a user, you can build new blocks yourself and throw them in the mix to fulfill your creative vision. To push creativity, NAP is built to provide extremely fast iteration times.

Central to NAP are some key philosophies:

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

NAP Framework ships with many useful modules, including: a Vulkan 2D/3D render engine, an audio engine for music playback, recording and analysis, a data sequencer, an editor to author application content, a system for creating and loading presets, a video player powered by FFmpeg  and a Python programming interface.

NAP also has built in support for many common protocols and standards, including: WebSocket, MIDI, OSC, Artnet, Serial, EtherCAT, OpenCV and SQLite. NAP has been battle tested in production for years. For more information about NAP, how it is commonly used and what it can do for you, visit the [napframework](https://www.napframework.com) website.

<p align="center">
  <img src="https://www.napframework.com/png/Vulkan_170px_Dec16.png">
</p>

## Documentation

NAP documentation can be found online at [www.napframework.com/doxygen](https://www.napframework.com/doxygen/). Take note that the installation and project creation instructions on that website apply to the binary (compiled) NAP package only. Follow the instructions in this document to build and work against NAP Framework from source.

## Gallery

Visit [www.napframework.com](https://www.napframework.com/showcase) for more examples

![Between Mind and Matter, Nick Verstand](https://www.napframework.com/jpg/bmm_1280.jpg)
[Between Mind and Matter](http://www.nickverstand.com/) by Nick Verstand, Marcel Smit and 4DSOUND
![Shylight, Studio Drift](https://www.napframework.com/jpg/shylight_basel_1280.jpg)
[Shylight](https://www.studiodrift.com/work#/work/shylight/) by Studio Drift
![Habitat, Heleen Blanken](https://www.napframework.com/jpg/habitat_1280.jpg)
[Habitat](https://www.heleenblanken.com/habitatbyheleenblanken) by Heleen Blanken, Naivi and Stijn van Beek
![4DSound System](https://www.napframework.com/jpg/4D_1280.jpg)
[4DSound System](https://4dsound.net/)
![NAP Framework](https://www.napframework.com/jpg/nap_heightmap_demo_1280.jpg)
[NAP Framework](https://napframework.com/) editor & demo

# Where to Start

Currently, whether working with the packaged framework release or against the framework source, we support the following operating systems:

- macOS Catalina (10.15)
- Ubuntu Linux LTS (20.04)
- Windows 10, Visual Studio 2019 (v142)

NAP's official releases are provided as binary packages at [www.napframework.com](https://napframework.com) and for most developers this is the best place to start. Our developer experience is tuned to work with the releases there, where the process of managing projects and modules is streamlined. Certain functionality, eg. packaging a project for distribution, is also only available through a packaged framework release. 
When working against a binary package, follow the official [installation](https://www.napframework.com/doxygen/) instructions, instead of the instructions in this document. Continue reading below to compile and get started with the NAP source code.

# Compilation

Windows 10<br/>
[![Build Status](http://engine9.nl:8092/app/rest/builds/buildType:(id:Public_PackageNapWin64)/statusIcon)](httphttp://engine9.nl:8092/viewType.html?buildTypeId=myID&guest=1)

Ubuntu 20.04<br/>
[![Build Status](http://engine9.nl:8092/app/rest/builds/buildType:(id:Public_PackageNapLinux)/statusIcon)](httphttp://engine9.nl:8092/viewType.html?buildTypeId=myID&guest=1)

macOS 10.15<br/>
[![Build Status](http://engine9.nl:8092/app/rest/builds/buildType:(id:Public_PackageNapOsx)/statusIcon)](httphttp://engine9.nl:8092/viewType.html?buildTypeId=myID&guest=1)

## Dependencies

To generate a solution and compile the source code you need to have installed: 

- [Qt 5](https://www.qt.io/download)
	- The precompiled package uses Qt 5.12.11 (LTS), although other versions are known to work.
	- Select **Downloads for open source users**.
	- During installation select **Custom installation** 
	- Filter on the **LTS** category to download Qt 5.12.11

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

`generate_solution.sh` to generate an `XCode project` (macOS Catalina/10.15 x86-64)<br>
`generate_solution.bat` to generate a `Visual Studio Solution` (Windows 10 x86-64)<br>
`generate_solution.sh` to generate `make files` (Ubuntu LTS Linux 20.04 x86-64)<br>

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

## Package for Desktop OS

To package NAP run: `package.bat` (Windows) or `package.sh` (macOS / Linux). You can prefix the environment variable for the location of your Qt Framework library if necessary, e.g.
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
