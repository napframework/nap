#!/usr/bin/env bash

cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P
../cmake/macos/universal/bin/cmake -Hsource -Bxcode -G Xcode -DCMAKE_INSTALL_PREFIX=macos/universal -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
cd xcode
xcodebuild -configuration Debug -target install
xcodebuild -configuration Release -target install
cd ..
install_name_tool -id @rpath/librttr_core.0.9.6.dylib macos/universal/bin/librttr_core.0.9.6.dylib
install_name_tool -id @rpath/librttr_core_d.0.9.6.dylib macos/universal/bin/librttr_core_d.0.9.6.dylib
