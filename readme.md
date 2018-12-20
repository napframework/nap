New Amsterdam Platform {#mainpage}
=======================

# Description

NAP (New Amsterdam Platform) is a platform that merges game technology with the flexibility of creative coding environments. The end result is a platform that offers both stability, ease of use and freedom. If you want to create dynamic content for high resolution screens or control an entire theatre stage: NAP is the tool for you.

NAP provides a single platform where you can use any protocol (such as OSC, MIDI or Art-Net) in conjunction with a 3D graphics engine to create real-time content that is transmitted to any device you like. NAP is built to scale up to a large number of output devices: many speakers, many lights, many lasers, many screens.
	
NAPs design is intended to be as open as possible: there is little fixed functionality, but there are a lot of useful blocks that can be tied together by a user to create the experience you desire. As a user, you can build new blocks yourself and throw them in the mix to fulfill your creative vision. To push creativity, NAP is built to provide extremely fast iteration times.

Central to NAP are a couple of key philosophies:

- NAP is completely data driven
- NAP is heavily influenced by modern game engine design, with one exception:
- NAP does not dictate any sort of pipeline, you decide how you render, couple devices and create sound etc.
- NAP applications are lean and mean, only package and ship what you need
- NAP is easy to extend: build your own modules and components
- NAP wants you to be safe and validates data for you
- NAP is responsive: hot-load content changes directly in to the running app
- NAP is completely cross-platform and supports all modern desktop environments
- NAP ships with many useful modules including: an OpenGL Renderer, OSC, MIDI, Art-Net, GUI, Audio etc.

## Why

Creative fields such as advertisement, film and the performing arts are starting to overlap. Projects are growing in scope to meet demand. Therefore the need for stable and performant interactive software is increasing. Software needs to run for years without having to string together multiple applications or environments. To meet expectations NAP offers you a set of handles to tackle the most demanding creative problems.

## Compilation

To generate a solution and compile the source code you need to have installed: 

- [QT5](http://download.qt.io/official_releases/qt/)
- Latest version of [CMAKE](https://cmake.org/download/)

NAP depends on various third party libraries. A set of compatible libraries can be downloaded from our github page. Put the thirdparty library directory next to the NAP source directory:

- ~/dev
	- ~/nap
	- ~/thirdparty

You can generate a Visual Studio solution by running generateVSSolution.bat and an Xcode project by running generateXCodeProject.sh

## Package

To package NAP for release run: package.bat or package.sh. The various options for packaging can be queried by adding the --help flag as an input argument to the script. Errors should be self-explanatory.

Packaging NAP requires that your Qt version is a build from [qt.io](http://download.qt.io/official_releases/qt/) and that the environment variable QT_DIR points to its location.

After packaging a new zip or folder is created called: NAP-'X'-'Platform'-'Timestamp'. This package can be distributed and includes all demos, projects (if selected for packaging), modules and core components.

## Want to know more?

Take a look at our high level [documentation](https://www.napframework.com/doxygen/index.html) or [download](https://www.napframework.com) the latest version of NAP right now!	
