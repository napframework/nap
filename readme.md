# Nieuw Amsterdams Platform


== Clang-Format ==

In order to auto-format your code please use clang format. 
Using the following command line, it will find the .clang-format file in the root of the project and take its settings. 

C:\Program Files\LLVM\bin\clang-format.exe -i path/to/mysource.cpp.h -style=file

You can easily set up your IDE to insert the currently open source file where the file name should be.


== Structure ==

The core dir contains both the RTTI system and NAP engine. The modules dir contains NAP extensions

The napkin dir contains the NAP visual editor


== Third Party Dependencies ==

core has no dependencies and builds as is, both on Linux, Windows and OSX

modules have various dependencies, see readme in module dir for more information

napkin uses QT 5.X

dependencies SHOULD NOT be included in this repository 
