<br>
<p align="center">
  <img width=384 src="https://download.nap-labs.tech/identity/svg/logos/nap_logo_blue.svg">
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
	*	[Work Against Source](#work-against-source)
	*	[Package](#build-distributable-nap-package)
*	[Contributing](#contributing)
*	[License](#license)
	
# Description

[NAP](https://nap.tech) is an [open source](https://github.com/napframework), low overhead, real-time control & visualization plaform. Create fast, modular and responsive applications to interact with the world around you. NAP is built to scale up to a large number of input and output devices: many displays, many lights, many speakers, many sensors, many servos.

## Features

NAP Framework ships with many useful modules, including: a Vulkan 2D/3D render engine, a Vulkan Compute module, a multi-channel audio engine for music playback, recording and analysis, a sequencer to control parameters in real-time, an editor to author application content, a web-portal to control and monitor NAP applications in a browser, a system for creating and loading presets, a video player powered by FFmpeg and a Python programming interface.

NAP also has built in support for many common protocols and standards, including: WebSocket, MIDI, OSC, Artnet, Serial, EtherCAT, OpenCV and SQLite. NAP has been battle tested in production for years. For more information about NAP, how it is commonly used and what it can do for you, visit the [nap-framework.tech](https://nap-framework.tech) website.

<br>
<p align="center">
  <img width=256 img src="https://docs.nap-framework.tech/content/vulkan-logo.svg" >
</p>

## Philosophy

NAP is completely data driven and heavily influenced by modern game engine design, with one exception: it does not dictate any sort of pipeline. This allows NAP to run on any type of device: from low-power, energy efficient ARM computers such as the Raspberry Pi to industrial PCs and x86 gaming rigs. 

NAP applications are lean and mean: only package and ship what you actually use. On top of that NAP is easy to extend: build you own modules, resources, devices and components. NAP wants you to be safe and validates data for you on initialization. Applications are also responsive: hot-load content changes directly in to the running application. On top of that NAP is completely cross-platform and supports all modern desktop environments.

## Documentation

NAP documentation can be found online at [docs.nap-framework.tech](https://docs.nap-framework.tech/pages.html). Take note that the installation instructions on that website apply to the binary (compiled) NAP package only. Follow the instructions in this document to build and work against NAP Framework from source.

## Gallery

Visit [nap-labs.tech](https://nap-labs.tech/use-cases) for more examples

![Between Mind and Matter, Nick Verstand](https://download.nap-labs.tech/shared/bmm_1280.jpg)
[Between Mind and Matter](http://www.nickverstand.com/) by Nick Verstand, Marcel Smit and 4DSOUND
![Habitat, Heleen Blanken](https://download.nap-labs.tech/shared/habitat_1280.jpg)
[Habitat](https://www.heleenblanken.com/habitatbyheleenblanken) by Heleen Blanken, Naivi and Stijn van Beek
![Shylight, Studio Drift](https://download.nap-labs.tech/shared/shylight_basel_1280.jpg)
[Shylight](https://www.studiodrift.com/work#/work/shylight/) by Studio Drift
![4DSound System](https://download.nap-labs.tech/shared/4D_1280.jpg)
[4DSound System](https://4dsound.net/)
![NAP Framework](https://download.nap-labs.tech/shared/napkin_compute_1280.jpg)
[NAP Framework](https://nap.tech) editor & demo

# Where to Start

## System Compatibility

Currently, whether working with the packaged framework release or against the framework source, we support the following architectures and operating systems:

**x86**
```
x86-64: Windows (10 & 11), Visual Studio 2019 - MSVC
x86-64: Ubuntu Linux LTS (v20.04, v22.04 & v24.04) - GCC
```
**ARM**
```
armhf: Raspberry Pi OS (v11) - GCC
arm64: Ubuntu Linux LTS (v22.04) *experimental* - GCC
```

## Binary Packages

Pre-compiled packages of official NAP releases are made available for download on [Github](https://github.com/napframework/nap/releases) for all supported platforms. Follow the [framework installation instructions](https://docs.nap-framework.tech/pages.html) to get started. Continue reading below to compile, package and work with NAP from source.

## Raspberry Pi

Only the `Raspberry Pi 4` running `Debian Bullseye (v11, armhf)` is 'fully' supported. Headless applications and services without graphics should run on older models, although this has not been tested. The editor (napkin) only works on the Raspberry Pi 4 and higher.

# Compilation

## Dependencies

The editor (Napkin) depends on QT:

- The precompiled package uses Qt 6.5 (LTS), although other versions are known to work.
- Go to [qt.io](https://www.qt.io/download-open-source) for open source users
- Download the Qt online installer
- During installation select **Custom installation** 
- Filter on the **LTS** category to download and install Qt 6.5 for your target platform

Create an environment variable called `QT_DIR` and point it to the directory that contains the QT libraries, for example: `C:\qt\5.15.17\msvc2015_64`. The build system uses this environment variable to locate QT. Note that only the editor (Napkin) depends on Qt, NAP distributable applications do not have a dependency on Qt.

## Create the Solution

Run:

`check_build_environment` to ensure your build environment is up to date.

On success, run:

`generate_solution.bat` to generate a `Visual Studio Solution` (Windows)<br>
`generate_solution.sh` to generate `make files` (Linux)<br>

The solution allows you to build every target and inspect the code of the demos, editor, modules, core etc. NAP uses a pre-bundled version of CMake in third-party to ensure compatibility for all platforms. The default build configuration is `Release`. Alternatively you can use `CLion`.

## Run a Demo

Open the generated solution in `Visual Studio`, select a build configuration (`Debug`or `Release`) and a demo as target. Compile and run the demo. You can also use the `build` script to compile one or more projects using the command line, for example: `./build.sh helloworld`.

---

## Work Against Source

Allows you to step into the NAP Framework source code and make changes if required.

### Create Application

Run `tools/create_app` to create a new application:
```
./create_app.sh myApp
```

This creates a new application called `myApp` in the `apps` directory. It also adds `myApp` to `solution_info.json` in the root and regenerates the solution. Run the `build` script inside the application directory to compile it:

```
./build.sh
```

### Create Module

Run `tools/create_module` to create a new user module:
```
./create_app.sh myModule
```

This creates a new module called `napmyModule` in the `modules` directory. It also adds `napmyModule` to `solution_info.json` in the root and regenerates the solution. You can inlcude this module by adding it to the `RequiredModules` property in `app.json` or `module.json` files.

Note that when you make manual changes to the build system (by editing CMake or JSON) you must run `generate_solution` to update the solution.

### Download Module

A list of publicly available user modules can be found at [modules.nap-framework.tech](https://modules.nap-framework.tech). These modules are created, maintained and shared by NAP users, independent of NAP Framework.

### Share Your Module

You can share your module with other NAP users by making it available on [modules.nap-framework.tech](https://modules.nap-framework.tech). Follow the instructions [here](https://github.com/napframework/nap-modules#register-your-module) to register it.

---

## Build Distributable NAP Package

A packaged version of NAP will include all of the following:

**compiled object code** (headers & binaries)
* core engine
* system modules
* editor

**source code** (not compiled)
* demos
* user modules
* apps

After packaging a new zip or folder is created, with the naming convention `NAP-*Version*-*Platform*-*Timestamp*` (Timestamp may be optionally omitted). 

### Package

Run `package` to package NAP:
```
./package.sh
```

Alternatively, you can use the `-sna` flag to build a package that includes only one app, e.g.:
```
./package.sh -sna myApp
```

Some other useful flags:
* `-nt`: remove the timestamp
* `-nz`: do not create a zip file from the release
* `-ds`: include debug symbols. On Windows .pdb files are also packaged.

More options for packaging can be queried by adding the flag `--help` when running the script.

# Contributing

We welcome contributions and potential bug fixes. But before you submit any code for review make sure to read and follow our [C++ styleguide](styleguide/styleguide.md). Also take into consideration that reviewing code takes time: Be as thorough and explicit as possible. 

Use the github [issues](https://github.com/napframework/nap/issues) page for bug reports and well defined feature requests. Use the Github [discussions](https://github.com/orgs/napframework/discussions) page for general questions and help.

New modules are not considered unless useful, vital or important enough to have as part of the core release. If you feel a module is missing we would like to [hear](https://github.com/orgs/napframework/discussions) from you. If a module depends on a third-party library, linkage should be dynamic and not violate the NAP license policy. Static linkage is discouraged unless recommended by the library or when a NAP application, that uses the module, doesn't require the library to link and run. In that case all third-party code is compiled into the module when NAP is packaged. Third-party dependencies must work cross-platform and must be compiled using
```
MSVC, Platform Toolset v142 on Windows 10
GCC <= 9.3.0 on Ubuntu LTS 20.04
```

# License

NAP Framework is open source software, licensed under the [Mozilla Public License Version 2.0](https://www.mozilla.org/en-US/MPL/2.0/). You can use the NAP source code and NAP distributable package to create both commercial and non-commercial derivate works, as long as the MPL 2.0 license is not violated. 

This means that you must make your changes to the NAP source code available under MPL. You can combine NAP source code with proprietary code, as long as you keep the NAP source code in separate files. Version 2.0 is, by default, compatible with LGPL and GPL version 2 or greater. You can also distribute binaries under a proprietary license, as long as you make the NAP source code available under MPL. 

In short: You are allowed to use and modify the NAP source code, for both commercial and non commercial purposes. However: if you make changes to the NAP source code, you have to make those changes available for others to use. We encourage people to share their ideas with the community. Keeping knowledge to yourself is not what NAP Framework is about.
