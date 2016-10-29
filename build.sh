#!/usr/bin/env bash
rm -R build
cmake -H. -Bbuild -DCMAKE_PREFIX_PATH="~/Qt/5.7/gcc_64;~/Documents/rapidjson"