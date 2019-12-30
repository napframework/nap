Project Structure
=================

NAP is built in two possible structures.
The one of the largest differences between the source build system and framework release build system is that the framework release system is project-centric, having an IDE/makefile solution per project, instead of one that covers everything. Plus, where possible, we attempt to not copy everything all over the place. So that means on Linux on macOS NAP modules and user modules are sourced from **/modules/<module name>/lib/<build type>** and the same for **/user_modules**.


Source Build
------------
**Unix/OSX**<br>
Binaries:<br>
/bin/{BUILD_CONFIG}<br>

Modules:<br>
/lib/{BUILD_CONFIG}

**Windows**<br>
Everything:<br>
/bin/{BUILD_CONFIG}

Framework Release
-----------------
**Unix/OSX**<br>
Project exe and Napkin:<br>
/projects/{PROJECT}/bin/{BUILD_CONFIG}
 
NAP modules:<br>
/modules/{MODULE}/lib/{BUILD_CONFIG}

User modules:<br>
/user_modules/{MODULE}/lib/{BUILD_CONFIG}
 
Project module (if it exists):<br>
/projects/{PROJECT}/module/lib/{BUILD_CONFIG}

**Windows**
Everything:<br>
/projects/{PROJECT}/bin/{BUILD_CONFIG}

Packaged App
------------
**Unix/OSX**<br>
Executables:<br>
/

Modules:<br>
/lib

**Windows**<br>
Everything:<br>
/