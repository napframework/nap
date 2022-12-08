#!/bin/bash
echo "Build static and dynamic library for C++"
echo "========================================"
cd Binaries
make $1 || exit 1
cd ../
echo -e "done"


echo "Build C++ Examples"
echo "=================="
for d in Examples/*
do
echo "Build "$d
cd $d
make $1 || exit 1
cd ../../
done
echo -e "All examples successfully built"