# Qt for NAP

When working from source NAP requires Qt v5.15.2 which for most major platforms can be downloaded from [qt.io](https://qt.io). Once deployed Qt should then be pointed to with the environment `QT_DIR` so it can be picked up by NAP.

eg. on Linux for x86_64 in your .bashrc this might look like 
```
export QT_DIR=/home/username/Qt/5.11.3/gcc_64
```

For published NAP Framework Releases Qt is included within and doesn't need to be downloaded separately.

## Linux ARM builds

We've rolled our own Qt builds for Linux ARMhf (Raspbian Buster on Raspberry Pi v4) and ARM64 (Ubuntu 20.04) which are available here:

[https://UPDATE_TODO_THE_LINK](https://UPDATE_TODO_THE_LINK)

These should be deployed and identified with the environment variable as described above.

### Building for Linux ARM

These processes were developed for a Raspberry Pi 4 running Rasbian Buster and an ARM64 host running Ubuntu 20.04.

Environment preparation
```
sudo apt install build-essential libfontconfig1-dev libdbus-1-dev libfreetype6-dev libicu-dev libinput-dev libxkbcommon-dev libsqlite3-dev libssl-dev libpng-dev libjpeg-dev libglib2.0-dev ibx11-dev libxcb1-dev  libxext-dev libxi-dev libxcomposite-dev libxcursor-dev libxtst-dev libxrandr-dev libfontconfig1-dev libfreetype6-dev libx11-xcb-dev libxext-dev libxfixes-dev libxi-dev libxrender-dev libxcb1-dev  libxcb-glx0-dev  libxcb-keysyms1-dev libxcb-image0-dev  libxcb-shm0-dev libxcb-icccm4-dev libxcb-sync-dev libxcb-xfixes0-dev libxcb-shape0-dev  libxcb-randr0-dev  libxcb-render-util0-dev  libxcb-util0-dev  libxcb-xinerama0-dev  libxcb-xkb-dev libxkbcommon-dev libxkbcommon-x11-dev
```

Download and extract source
```
mkdir src
cd src
wget https://download.qt.io/official_releases/qt/5.15/5.15.2/single/qt-everywhere-src-5.15.2.tar.xz
tar xf qt-everywhere-src-5.15.2.tar.xz
```

#### Deploying mkspecs

The contents of `build-aux/` (located alongside this file) should be deployed into `qtbase/mkspecs` within the extracted Qt framework path. 

These are are based on [https://github.com/oniongarlic/qt-raspberrypi-configuration.git](https://github.com/oniongarlic/qt-raspberrypi-configuration.git) and other configurations, but not focusing on EGLFS.

#### Configuring for ARMhf / Raspberry Pi 4

```
sudo apt install libgles2-mesa-dev libgbm-dev libdrm-dev
mkdir build
cd build
PKG_CONFIG_LIBDIR=/usr/lib/arm-linux-gnueabihf/pkgconfig:/usr/share/pkgconfig \
../qt-everywhere-src-5.15.2/configure -platform linux-rpi4-v3d-g++ \
-v \
-opengl desktop \
-no-gtk \
-opensource -confirm-license -release \
-reduce-exports \
-force-pkg-config \
-nomake examples -no-compile-examples -nomake tests \
-skip qt3d \
-skip qtactiveqt \
-skip qtandroidextras \
-skip qtcanvas3d \
-skip qtcharts \
-skip qtdatavis3d \
-skip qtgamepad \
-skip qtgraphicaleffects \
-skip qtlocation \
-skip qtmacextras \
-skip qtpurchasing \
-skip qtquickcontrols \
-skip qtquickcontrols2 \
-skip qtscript \
-skip qtscxml \
-skip qtsensors \
-skip qtserialbus \
-skip qtserialport \
-skip qtspeech \
-skip qttools \
-skip qttranslations \
-skip qtvirtualkeyboard \
-skip qtwayland \
-skip qtwebengine \
-skip qtwebview \
-skip qtwinextras \
-skip qtx11extras \
-skip wayland \
-qt-pcre \
-no-pch \
-no-use-gold-linker \
-no-cups \
-ssl \
-evdev \
-system-freetype \
-system-libjpeg \
-system-libpng \
-system-zlib \
-fontconfig \
-glib \
-make libs \
-prefix /opt/Qt/5.15.2 \
-qpa xcb \
-dbus-linked \
-no-gbm \
-no-eglfs \
-no-linuxfb \
-recheck
```

#### Configuring for ARM64

```
mkdir build
cd build
../qt-everywhere-src-5.15.2/configure \
-platform linux-rpi64-vc4-g++ \
-v \
-opengl desktop \
-no-gtk \
-opensource -confirm-license -release \
-reduce-exports \
-force-pkg-config \
-nomake examples -no-compile-examples -nomake tests \
-skip qt3d \
-skip qtactiveqt \
-skip qtandroidextras \
-skip qtcanvas3d \
-skip qtcharts \
-skip qtdatavis3d \
-skip qtgamepad \
-skip qtgraphicaleffects \
-skip qtlocation \
-skip qtmacextras \
-skip qtpurchasing \
-skip qtquickcontrols \
-skip qtquickcontrols2 \
-skip qtscript \
-skip qtscxml \
-skip qtsensors \
-skip qtserialbus \
-skip qtserialport \
-skip qtspeech \
-skip qttools \
-skip qttranslations \
-skip qtvirtualkeyboard \
-skip qtwayland \
-skip qtwebengine \
-skip qtwebview \
-skip qtwinextras \
-skip qtx11extras \
-skip wayland \
-qt-pcre \
-no-pch \
-no-use-gold-linker \
-no-cups \
-ssl \
-reduce-exports \
-evdev \
-system-freetype \
-system-libjpeg \
-system-libpng \
-system-zlib \
-fontconfig \
-glib \
-make libs \
-prefix /opt/Qt/5.15.2 \
-qpa xcb \
-dbus-linked \
-no-gbm \
-no-eglfs \
-no-linuxfb \
-recheck
```

#### Build

```
make -j4
sudo make install
```

Create the tarball from  `/opt/Qt/5.15.2`.

#### Notes

Cross compilation to ARMhf (Raspberry Pi 4) via a Docker image and QEMU repetitively failed. Bare metal is recommended for now.

The following were used in developing this process:

[https://www.tal.org/tutorials/building-qt-515-raspberry-pi](https://www.tal.org/tutorials/building-qt-515-raspberry-pi)

[https://github.com/koendv/qt5-opengl-raspberrypi](https://github.com/koendv/qt5-opengl-raspberrypi)

## Licenses

The licenses contained here are packaged into the NAP Framework Release when the Qt framework used doesn't contain an available source.