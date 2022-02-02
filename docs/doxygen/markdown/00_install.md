Install {#install}
=======================

*	[Overview](@ref build_env_overview)
*	[Windows](@ref win64)
	*	[Setup Your Build Environment](@ref setup_build_env_win64)
	*	[Run Your First Demo](@ref run_demo_win64)
*	[macOS](@ref macos)
	*	[Setup Your Build Environment](@ref setup_build_env_macos)
			*	[Manual Dependency Installation](@ref setup_manual_macos)
	*	[Run Your First Demo](@ref run_demo_macos)
*	[Linux](@ref linux) 
	*	[Desktop](@ref linux_desktop)
	*	[Raspberry Pi](@ref linux_pi)
	*	[Setup Your Build Environment](@ref setup_build_env_linux)
		*	[Manual Dependency Installation](@ref setup_manual_linux)
	*	[Run Your First Demo](@ref run_demo_linux)

# Overview {#build_env_overview}

Installing the NAP release involves extracting the archive and running our script to guide you through dependency installation. Projects reside within the framework folder structure, and as such we recommend extracting the release into a user directory instead of a system directory.  For example on Windows something like `My Documents` is a more suitable location than C:\\Program Files.

The prerequisites installation script is called `check_build_environment` and can be found in the tools directory within the release.  The script attempts to verify that your build environment is ready for NAP and is designed to be re-run until all checks are successfully passed.

# Windows {#win64}

## Setup Your Build Environment {#setup_build_env_win64}

NAP $(NAP_VERSION_FULL) supports `Windows 10` with `Visual Studio 2019 (v142)` . Although NAP is known to work on other versions of Windows, v10 is the current supported platform. Other versions of Visual Studio are not supported at this moment.

Download and install <a href="https://visualstudio.microsoft.com/downloads/" target="_blank">Visual Studio 2019</a>. This link points to the community edition which can be used for free. Make sure to select `Desktop development with C++` when installing Visual Studio, including `MSVC v142 - VS 2019 C++ x64/x86` and the `Windows 10 SDK`. Other settings are optional. Download and install the <a href=https://www.microsoft.com/en-us/download/details.aspx?id=40784 target="_blank">Visual C++ 2013 Redistributable (x64)</a>.

1. Extract `NAP-$(NAP_VERSION_FULL)-Win64.zip` using Explorer (or your preferred tool)
2. In `NAP-$(NAP_VERSION_FULL)-Win64\tools` run `check_build_environment.bat` to guide you through installing prerequisites, following the instructions

## Run Your First Demo {#run_demo_win64}

1. Navigate to `NAP-$(NAP_VERSION_FULL)-Win64\demos\helloworld` in Explorer
2. Run `regenerate.bat`
3. Open the generated Visual Studio solution (shown in Explorer)
4. Select the `Release` configuration
5. Click the run button

# macOS {#macos}

## Setup Your Build Environment {#setup_build_env_macos}

NAP $(NAP_VERSION_FULL) supports `macOS Catalina` with `XCode 11.7`. Although NAP is known to work on other macOS releases, Catalina is the current supported platform.

1. Extract the release by double clicking `NAP-$(NAP_VERSION_FULL)-macOS.zip` in Finder
2. Ctrl-click on `tools/unquarantine_framework.command` 
3. Select Open to remove the framework from <a href="https://en.wikipedia.org/wiki/Gatekeeper_(macOS)" target="_blank">macOS' Gatekeeper</a> quarantine
4. In `NAP-$(NAP_VERSION_FULL)-macOS/tools` run `check_build_environment` to guide you through installing prerequisites, following the instructions

Note: In a future release support for application bundles and codesigning will be assessed, however for the moment the script above is required to allow the framework to operate on a system with the Gatekeeper enabled. A similar script is included with packaged projects.

### Manual Dependency Installation {#setup_manual_macos}

Alternatively you can follow the steps below to install the dependencies, however we still recommend running `check_build_environment` afterwards to verify your build environment.

1. Install [Xcode](https://itunes.apple.com/us/app/xcode/id497799835?mt=12) from the App Store
2. Open Xcode once to accept the license agreement
3. Install Xcode Command Line Tools by running the following command in a terminal:
```    
xcode-select --install
```

## Run Your First Demo {#run_demo_macos}

1. Navigate to `NAP-$(NAP_VERSION_FULL)-macOS/demos/helloworld` in Finder
2. Run `regenerate`
3. Open the generated Xcode project
4. Click on `edit scheme`
5. Select `helloworld` as Executable & `Release` as Build Configuration
6. Close the dialog and click the `run` button

# Linux {#linux}

## Raspberry Pi {#linux_pi}

Only the `Raspberry Pi 4` running `Debian Bullseye (v11, armhf)` is 'fully' supported. Applications that make use of the render engine will not run on older Raspberry Pi models because there is no support for Vulkan. However, headless applications without graphics might work on older Raspberry Pi models.

## Desktop {#linux_desktop}

NAP $(NAP_VERSION_FULL) supports `Ubuntu Linux 20.04` on `x86-64` machines using `GCC`. Although NAP is known to run on other distros: `Ubuntu 20.04 x86_64` is currently the only supported Linux desktop environment.

## Setup Your Build Environment {#setup_build_env_linux}

1. Extract the release:
```
tar jxvf NAP-0.5.0-Linux.tar.bz2
```
2. Run the `check_build_environment` script to guide you through installing prerequisites, following the instructions:
```
cd NAP-0.5.0-Linux
./tools/check_build_environment
```
3. If any changes are required re-run `check_build_environment` after those changes have been made to verify the final environment

### Manual Dependency Installation {#setup_manual_linux}

Alternatively you can follow the steps below to install the dependencies, however we still recommend running `check_build_environment` afterwards to verify your build environment.

1. Run the following to install the dependencies via apt:
```
apt-get install build-essential patchelf libglu1-mesa-dev
```

## Run Your First Demo {#run_demo_linux}

1. Navigate to the helloworld demo
```
cd NAP-0.5.0-Linux/demos/helloworld
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

This example uses `make files`. Instead of using `make files` you can use [Visual Studio Code](https://code.visualstudio.com/) or [CLion](https://www.jetbrains.com/clion). Although we don't officially support these environments others have confirmed that they work as expected. Both can be configured using CMake.