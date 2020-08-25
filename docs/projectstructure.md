Output Paths
=================

When working in the Source context NAP uses a different directory and build system structure to that in the Framework Release. One of the largest differences between the two is that the Framework Release system is project-centric, having an IDE/makefile solution per project, instead of one that covers everything. Plus, where possible, we attempt to not copy everything all over the place. So that means on Linux and macOS NAP modules and user modules are deployed to a location relative to the module.

See below for a summary of the location of our compiled objects across the contexts and platforms.

Source context
------
**Linux/macOS**<br>
Executables:<br>
/bin/{BUILD_CONFIG}<br>

Modules:<br>
/lib/{BUILD_CONFIG}

**Windows**<br>
Everything:<br>
/bin/{BUILD_CONFIG}

Framework Release context
------
**Linux/macOS**<br>
Executables:<br>
/projects/{PROJECT_NAME}/bin/{BUILD_TYPE}
 
NAP modules:<br>
/modules/{MODULE_NAME}/lib/{BUILD_TYPE}

User modules<br>
/user_modules/{MODULE_NAME}/lib/{BUILD_TYPE}
 
Project modules (when they exist):<br>
/projects/{PROJECT_NAME}/module/lib/{BUILD_TYPE}

**Windows**<br>
Everything:<br>
/projects/{PROJECT_NAME}/bin/{BUILD_TYPE}

Packaged App context
------
**Linux/macOS**<br>
Executables:<br>
/

Modules:<br>
/lib

**Windows**<br>
Everything:<br>
/

Napkin Path
=================

These are locations for Napkin in the three contexts (which remain the same across platforms):

Source context:<br>
/bin/{BUILD_CONFIG}/napkin

Framework Release context:<br>
/tools/napkin

Packaged App context:<br>
/napkin

