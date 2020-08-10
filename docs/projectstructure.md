Project Structure
=================

NAP is built in two possible structures.
The one of the largest differences between the Source build system and Framework Release build system is that the Framework Release system is project-centric, having an IDE/makefile solution per project, instead of one that covers everything. Plus, where possible, we attempt to not copy everything all over the place. So that means on Linux and macOS NAP modules and user modules are sourced from **/modules/<module name>/lib/<build type>** and the same for **/user_modules**.


Source
------
**Linux/macOS**<br>
Binaries:<br>
/bin/{BUILD_CONFIG}<br>

Modules:<br>
/lib/{BUILD_CONFIG}

**Windows**<br>
Everything:<br>
/bin/{BUILD_CONFIG}

Framework Release
-----------------
**Linux/macOS**<br>
Project exe and Napkin:<br>
/projects/{PROJECT}/bin/{BUILD_TYPE}
 
NAP modules:<br>
/modules/{MODULE}/lib/{BUILD_TYPE}

User modules:<br>
/user_modules/{MODULE}/lib/{BUILD_TYPE}
 
Project module (if it exists):<br>
/projects/{PROJECT}/module/lib/{BUILD_TYPE}

**Windows**
Everything:<br>
/projects/{PROJECT}/bin/{BUILD_TYPE}

Packaged App
------------
**Linux/macOS**<br>
Executables:<br>
/

Modules:<br>
/lib

**Windows**<br>
Everything:<br>
/
