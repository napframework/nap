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

Installing the NAP beta release involves extracting the archive and running our script to guide you through dependency installation.

Projects within the NAP beta reside within the framework folder structure, and as such we recommend extracting the release into a user directory instead of a system directory.  For example on Windows something like My Documents is a more suitable location than C:\\Program Files.

The prerequisites installation script is called `check_build_environment` and can be found in the tools directory within the release.  The script attempts to verify that your build environment is ready for NAP and is designed to be re-run until all checks are successfully passed.

# Setup Your Build Environment {#setup_build_env}

## Windows {#setup_build_env_win64}

NAP $(NAP_VERSION_FULL) supports Windows 10 with Visual Studio 2015. Although NAP is known to work on other versions of Windows, v10 is the current supported platform. Other versions of Visual Studio are not supported at this moment.

Follow these steps for a guided installation:
1. Extract `NAP-$(NAP_VERSION_FULL)-Win64.zip` using Explorer (or your preferred tool)
2. In `NAP-$(NAP_VERSION_FULL)-Win64\tools` run `check_build_environment.bat` to guide you through installing prerequisites, following the instructions

_Manual Dependency Installation_

Alternatively you can follow the steps below to install the dependencies, however we still recommend running `check_build_environment.bat` afterwards to verify your build environment.

1. Download and install <a href="https://go.microsoft.com/fwlink/?LinkId=532606&clcid=0x409" target="_blank">Visual Studio 2015</a>. This link points to the community edition which can be used for free. Make sure to select `Visual C++` under `programming languages` when installing Visual Studio, including `Common Tools for Visual C++ 2015` and `Microsoft Foundation Classes for C++`. 

## macOS {#setup_build_env_macos}

NAP $(NAP_VERSION_FULL) supports macOS Catalina with XCode 10.5. Although NAP is known to work on other macOS releases, Catalina is the current supported platform. Other versions of XCode are not supported at this moment.

Follow these steps for a guided installation:
1. Extract the release by double clicking `NAP-$(NAP_VERSION_FULL)-macOS.zip` in Finder
2. In `NAP-$(NAP_VERSION_FULL)-macOS/tools` run `check_build_environment` to guide you through installing prerequisites, following the instructions

_Manual Dependency Installation_

Alternatively you can follow the steps below to install the dependencies, however we still recommend running `check_build_environment` afterwards to verify your build environment.

1. Install <a href="https://itunes.apple.com/us/app/xcode/id497799835?mt=12" target="_blank">Xcode from the App Store</a>
2. Open Xcode once to accept the license agreement.  You can close it afterwards.
3. Install Xcode Command Line Tools by running the following command in a terminal:
```    
xcode-select --install
```

## Linux {#setup_build_env_linux}

NAP $(NAP_VERSION_FULL) supports Ubuntu Linux 18.04 on x86-64 machines using GCC. Although NAP is known to work on other Ubuntu releases, 18.04 is the current supported version.

Follow these steps for a guided installation:
1. Extract the release:
```
tar jxvf NAP-0.3.2-Linux.tar.bz2
```
2. Run the `check_build_environment` script to guide you through installing prerequisites, following the instructions:
```
cd NAP-0.3.2-Linux
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
cd NAP-0.3.2-Linux/demos/helloworld
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
cd ../bin/*Debug*
./helloworld
```