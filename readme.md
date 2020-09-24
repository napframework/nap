New Amsterdam Platform
=======================

# Description

NAP (New Amsterdam Platform) is a platform that merges game technology with the flexibility of creative coding environments. The end result is a platform that offers both stability, ease of use and freedom. If you want to create dynamic content for high resolution screens or control an entire theatre stage: NAP is the tool for you.

NAP provides a single platform where you can use any protocol (such as OSC, MIDI, Art-Net, Websockets) in conjunction with a Vulkan 3D graphics engine to create real-time content that is transmitted to any device you like. NAP is built to scale up to a large number of output devices: many speakers, many lights, many lasers, many screens.
	
NAPs design is intended to be as open as possible: there is little fixed functionality, but there are a lot of useful blocks that can be tied together by a user to create the experience you desire. As a user, you can build new blocks yourself and throw them in the mix to fulfill your creative vision. To push creativity, NAP is built to provide extremely fast iteration times.

Central to NAP are a couple of key philosophies:

- NAP is completely data driven
- NAP is heavily influenced by modern game engine design, with one exception:
- NAP does not dictate any sort of pipeline, you decide how you render, couple devices and create sound etc.
- NAP applications are lean and mean, only package and ship what you need
- NAP is easy to extend: build your own modules, resources, devices and components
- NAP wants you to be safe and validates data for you
- NAP is responsive: hot-load content changes directly in to the running app
- NAP is completely cross-platform and supports all modern desktop environments
- NAP ships with many useful modules including: a Vulkan Renderer, Audio, Sequencer, OSC, MIDI, Art-Net, GUI, Audio, Websockets etc.
- NAP app content can be edited using an editor

## Why

Creative fields such as advertisement, film and the performing arts are starting to overlap. Projects are growing in scope to meet demand. Therefore the need for stable and performant interactive software is increasing. Software needs to run for years without having to string together multiple applications or environments. To meet expectations NAP offers you a set of handles to tackle the most demanding creative problems.

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

You can generate a solution by running `generate_solution.bat` or `generate_solution.sh`. NAP uses a pre-bundled version of CMake in third-party to ensure compatibility for all platforms.

---

## OPTION #1: Compile your project against NAP source
### Motivation
Compiling your own project allows you to debug your project and step into the NAP framework source code as well, when necessary.

### Process
* To see how you set up an app in source, look at the demos in the `demos` folder.
* Add your project to the main `CMakeLists.txt` file

 Running `./generate_solution.sh` will create the Xcode project for the entire NAP source. You can see the code for napkin, the demos, modules, etc.

---

## OPTION #2: Build your own NAP distribution package
A packaged version of NAP will include all of the following:
* core components
* standard modules
* demos
* your own project(s) - only if specified

After packaging a new zip or folder is created, with the naming convention `NAP`-*Version*-*Platform*-*Timestamp* (Timestamp may be optionally omitted).

**By default only headers and binaries are included; source code will be excluded.**

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
