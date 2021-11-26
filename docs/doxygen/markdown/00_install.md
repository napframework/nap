Install {#install}
=======================

*	[Overview](@ref build_env_overview)
*	[Setup Your Build Environment](@ref setup_build_env)
	*	[Windows](@ref setup_build_env_win64)
	*	[macOS](@ref setup_build_env_macos)
	*	[Linux](@ref setup_build_env_linux)
*	[Run Your First Demo](@ref run_demo)
	*	[Windows](@ref run_demo_win64)
	*	[macOS](@ref run_demo_macos)
	*	[Linux](@ref run_demo_linux)

# Overview {#build_env_overview}

Installing the NAP release involves extracting the archive and running our script to guide you through dependency installation.

Projects within the NAP beta reside within the framework folder structure, and as such we recommend extracting the release into a user directory instead of a system directory.  For example on Windows something like My Documents is a more suitable location than C:\\Program Files.

The prerequisites installation script is called `check_build_environment` and can be found in the tools directory within the release.  The script attempts to verify that your build environment is ready for NAP and is designed to be re-run until all checks are successfully passed.

# Setup Your Build Environment {#setup_build_env}

## Windows {#setup_build_env_win64}

NAP $(NAP_VERSION_FULL) supports Windows 10 with Visual Studio 2019 (v142) . Although NAP is known to work on other versions of Windows, v10 is the current supported platform. Other versions of Visual Studio are not supported at this moment. 

Download and install <a href="https://visualstudio.microsoft.com/downloads/" target="_blank">Visual Studio 2019</a>. This link points to the community edition which can be used for free. Make sure to select `Desktop development with C++` when installing Visual Studio, including `MSVC v142 - VS 2019 C++ x64/x86` and the `Windows 10 SDK`. Other settings are optional. 

Download and install the <a href=https://www.microsoft.com/en-us/download/details.aspx?id=40784 target="_blank">Visual C++ 2013 Redistributable (x64)</a>.

Follow these steps for a guided installation:
1. Extract `NAP-$(NAP_VERSION_FULL)-Win64.zip` using Explorer (or your preferred tool)
2. In `NAP-$(NAP_VERSION_FULL)-Win64\tools` run `check_build_environment.bat` to guide you through installing prerequisites, following the instructions

## macOS {#setup_build_env_macos}

NAP $(NAP_VERSION_FULL) supports macOS Catalina with XCode 11.7. Although NAP is known to work on other macOS releases, Catalina is the current supported platform. Other versions of XCode are not supported at this moment.

Follow these steps for a guided installation:
1. Extract the release by double clicking `NAP-$(NAP_VERSION_FULL)-macOS.zip` in Finder
2. Ctrl-click on `tools/unquarantine_framework.command` and select Open to remove the framework from <a href="https://en.wikipedia.org/wiki/Gatekeeper_(macOS)" target="_blank">macOS' Gatekeeper</a> quarantine (see note below)
3. In `NAP-$(NAP_VERSION_FULL)-macOS/tools` run `check_build_environment` to guide you through installing prerequisites, following the instructions

Note: In a future release support for application bundles and codesigning will be assessed, however for the moment the script above is required to allow the framework to operate on a system with the Gatekeeper enabled. A similar script is included with packaged projects.

_Manual Dependency Installation_

Alternatively you can follow the steps below to install the dependencies, however we still recommend running `check_build_environment` afterwards to verify your build environment.

1. Install <a href="https://itunes.apple.com/us/app/xcode/id497799835?mt=12" target="_blank">Xcode from the App Store</a>
2. Open Xcode once to accept the license agreement.  You can close it afterwards.
3. Install Xcode Command Line Tools by running the following command in a terminal:
```    
xcode-select --install
```

## Linux {#setup_build_env_linux}

NAP $(NAP_VERSION_FULL) supports Ubuntu Linux 20.04 on x86-64 machines using GCC. Although NAP is known to work on other Ubuntu releases, 20.04 is the current supported version.

Follow these steps for a guided installation:
1. Extract the release:
```
tar jxvf NAP-0.4.5-Linux.tar.bz2
```
2. Run the `check_build_environment` script to guide you through installing prerequisites, following the instructions:
```
cd NAP-0.4.5-Linux
./tools/check_build_environment
```
3. If any changes are required re-run `check_build_environment` after those changes have been made to verify the final environment

_Manual Dependency Installation_

Alternatively you can follow the steps below to install the dependencies, however we still recommend running `check_build_environment` afterwards to verify your build environment.

1. Run the following to install the dependencies via apt:
```
apt-get install build-essential patchelf libglu1-mesa-dev
```

# Run Your First Demo {#run_demo}

Now for the fun part.  Let's get the HelloWorld demo up and running.

All demonstration projects can be found within the demos folder within the release.

## Windows {#run_demo_win64}

1. Navigate to `NAP-$(NAP_VERSION_FULL)-Win64\demos\helloworld` in Explorer
2. Run `regenerate.bat`
3. Open the Visual Studio solution which will be generated and shown in Explorer
4. Click the run button

## macOS {#run_demo_macos}

1. Navigate to `NAP-$(NAP_VERSION_FULL)-macOS/demos/helloworld` in Finder
2. Run `regenerate`
3. Open the Xcode project which will be generated and shown in Finder
4. Use the Product > Scheme menu or the dropdown and select `helloworld`
5. Click the run button

## Linux {#run_demo_linux}

1. Navigate to the helloworld demo
```
cd NAP-0.4.5-Linux/demos/helloworld
```
2. Generate the Unix makefiles
```
./regenerate
```
3. Build the solution
```
cd build
make
```
4. Run the demo
```
cd ../bin/Debug
./helloworld
```
