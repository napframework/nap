#!bin/sh

echo Package NAP app.
echo Usage: package_macos_app [target] [build directory] [code signature]

# Check if target is specified
if [ "$#" -lt "1" ]; then
  echo "Specify a target."
  exit 0
fi

# Install jq utility to parse json
if [ "$(uname)" == "Darwin" ]; then
  brew install jq
fi

target=$1
if [ $# = "1" ]; then
  build_directory="build"
else
  build_directory=$2
fi

# Remove bin directory from previous builds
# This is important otherwise artifacts from previous builds could be included in the app installation
echo Cleaning previous build output...
rm -rf $build_directory/bin

# Generate the build directory
cmake -S . -B $build_directory

# Build the specified target
cmake --build $build_directory --target $target --config Debug --parallel 8

# Install to app bundle
if [ "$(uname)" == "Darwin" ]; then
  # Add app bundle file extension on MacOS
  install_location=install/`jq -r '.Title' build/bin/$target.json`.app
else
  install_location=install/`jq -r '.Title' build/bin/$target.json`
fi

# Cleaning previous install, if any
echo Cleaning previous install output...
rm -rf $install_location

# Run cmake install process
cmake --install build --prefix $install_location

# Codesign MacOS app bundle
if [ "$(uname)" == "Darwin" ]; then
  if [ "$#" -lt "3" ]; then
    codesign --deep -s - -f $install_location
  else
    codesign --deep -s "$3" -f -i "com.$target.napframework.www" -f $install_location
  fi
fi

# Remove the build directory if it wasn't specified
if [ $# = "1" ]; then
  echo Removing build directory...
  rm -rf build
fi