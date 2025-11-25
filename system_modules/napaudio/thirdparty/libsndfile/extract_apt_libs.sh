#!/usr/bin/env bash
# Usage: ./extract_apt_libs.sh <platform> 
# Example: ./extract_apt_files.sh x86_64 
set -euo pipefail
IFS=$'\n\t'

if [ $# -lt 1 ]; then
  echo "Usage: $0 <platform>"
  exit 1
fi

extract_apt_files() {
  if [ $# -lt 3 ]; then
    echo "Usage: extract_apt_files <package_name> <platform> <dest_dir>"
    exit 1
  fi

  local PKG="$1"
  local PLATFORM="$2"
  local ROOT_DEST="$3"

  if [ -z "$PKG" ]; then
    echo "Package name is empty"
    exit 1
  fi

  local CLEAN_PKG_NAME="${PKG/-dev/}"
  local DEST_DIR="$ROOT_DEST/$CLEAN_PKG_NAME/linux/$PLATFORM"
  local INC_DIR="$DEST_DIR/include"
  local LIB_DIR="$DEST_DIR/lib"

  mkdir -p "$INC_DIR" "$LIB_DIR"

  echo "Collecting installed files for package: $PKG"
  # dpkg -L lists all paths the package owns (dirs and files)
  local FILES
  if ! FILES="$(dpkg -L "$PKG" 2>/dev/null | grep '^/')"; then
    echo "dpkg -L failed for $PKG (not installed or wrong name?)"
    exit 1
  fi

  if [ -z "$FILES" ]; then
    echo "No files found for $PKG"
    exit 1
  fi

  echo "Copying headers to $INC_DIR and shared libs to $LIB_DIR ..."

  # We’ll copy any directory that *is* an include dir entirely,
  # and for single header files under .../include/... we’ll strip up to 'include/'.
  # For libs, copy *.so and *.so.* into $LIB_DIR (keep symlinks).
  while IFS= read -r f; do
    
    # Headers: files under an include path
    if [ -f "$f" ] && [[ "$f" == /usr/include/*.h ]]; then
      rel="${f#*/include/}"                 # strip everything up to 'include/'
      dest="$INC_DIR/$rel"
      mkdir -p "$(dirname "$dest")"
      # echo "f: $f  dest: $dest rel: $rel"
      cp -a "$f" "$dest"
      continue
    fi

    # Shared libs: regular files or symlinks that look like .so or .so.*
    if [ -e "$f" ] && [[ "$f" == *.so  ]]; then
      # Put the file/symlink directly into lib/ (flatten)
      shopt -s nullglob
      for a in "$f"*; do
        cp -a "$a" "$LIB_DIR"/
        # echo "----a: $a libdir: $LIB_DIR"
      done

      continue
    fi

    if [ -f "$f" ] && [[ "$f" == *copyright ]]; then
      cp "$f" "$DEST_DIR"/
      continue
    fi

  done <<< "$FILES"

  # echo "✅ Done: $DEST_DIR"
}

PLATFORM=$1
DEST="$( cd "$( dirname "${BASH_SOURCE[0]}/.." )" && pwd )"



#install the libs if not installed already.
# sudo apt update
# sudo apt install libflac-dev libogg-dev libvorbis-dev libopus-dev libmpg123-dev libmp3lame-dev


# Add/remove packages here
extract_apt_files "libflac-dev"   "$PLATFORM" "$DEST"
extract_apt_files "libogg-dev"    "$PLATFORM" "$DEST"
extract_apt_files "libvorbis-dev" "$PLATFORM" "$DEST"
extract_apt_files "libopus-dev"   "$PLATFORM" "$DEST"
extract_apt_files "libmpg123-dev" "$PLATFORM" "$DEST"
extract_apt_files "libmp3lame-dev" "$PLATFORM" "$DEST"


#remove unnecesary libs and includes
# echo "${DEST}/libmpg123/linux/${PLATFORM}/lib/libsyn123.so"*
# echo "${DEST}/libmpg123/linux/${PLATFORM}/lib/libout123.so"*
# echo "${DEST}/libmpg123/linux/${PLATFORM}/include"/*/out123.h
# echo "${DEST}/libmpg123/linux/${PLATFORM}/include"/*/syn123.h

rm -f -- "${DEST}/libmpg123/linux/${PLATFORM}/lib/libsyn123.so"*
rm -f -- "${DEST}/libmpg123/linux/${PLATFORM}/lib/libout123.so"*
mkdir -p "${DEST}/libmpg123/linux/${PLATFORM}/include/libmpg123/"

if [[ "$PLATFORM" == "x86_64" ]] || [[ "$PLATFORM" == "arm64" ]]; then
  rm -f -- "${DEST}/libmpg123/linux/${PLATFORM}/include"/*/out123.h
  rm -f -- "${DEST}/libmpg123/linux/${PLATFORM}/include"/*/syn123.h
  mv "${DEST}/libmpg123/linux/${PLATFORM}/include"/*/*.h "${DEST}/libmpg123/linux/${PLATFORM}/include/libmpg123/"
else
  rm -f -- "${DEST}/libmpg123/linux/${PLATFORM}"/*/out123.h
  rm -f -- "${DEST}/libmpg123/linux/${PLATFORM}"/*/syn123.h
  mv "${DEST}/libmpg123/linux/${PLATFORM}"/*/fmt123.h "${DEST}/libmpg123/linux/${PLATFORM}/include/libmpg123/"
  mv "${DEST}/libmpg123/linux/${PLATFORM}"/*/mpg123.h "${DEST}/libmpg123/linux/${PLATFORM}/include/libmpg123/"
fi


# patch rpaths for dynamic libs
RPATH="\$ORIGIN/../../../../libflac/linux/${PLATFORM}/lib:\
\$ORIGIN/../../../../libmp3lame/linux/${PLATFORM}/lib:\
\$ORIGIN/../../../../libmpg123/linux/${PLATFORM}/lib:\
\$ORIGIN/../../../../libogg/linux/${PLATFORM}/lib:\
\$ORIGIN/../../../../libopus/linux/${PLATFORM}/lib:\
\$ORIGIN/../../../../libvorbis/linux/${PLATFORM}/lib:\$ORIGIN"

patchelf --set-rpath "$RPATH" "${DEST}/libsndfile/linux/${PLATFORM}/lib/libsndfile.so"

patchelf --set-rpath "\$ORIGIN/../../../../libogg/linux/${PLATFORM}/lib:\$ORIGIN" "${DEST}/libflac/linux/${PLATFORM}/lib/libFLAC.so"
patchelf --set-rpath "\$ORIGIN/../../../../libogg/linux/${PLATFORM}/lib:\$ORIGIN" "${DEST}/libvorbis/linux/${PLATFORM}/lib/libvorbis.so"
patchelf --set-rpath "\$ORIGIN/../../../../libogg/linux/${PLATFORM}/lib:\$ORIGIN" "${DEST}/libvorbis/linux/${PLATFORM}/lib/libvorbisfile.so"
patchelf --set-rpath "\$ORIGIN/../../../../libogg/linux/${PLATFORM}/lib:\$ORIGIN" "${DEST}/libvorbis/linux/${PLATFORM}/lib/libvorbisenc.so"



# find "$DEST" -name "*.so" | xargs ldd

