/*********************************************************************/
 *
 *      Y O C T O P U C E    L I B R A R Y    f o r    C + +
 *
 * - - - - - - - - - - - License information: - - - - - - - - - - -
 *
 *  Copyright (C) 2011 and beyond by Yoctopuce Sarl, Switzerland.
 *
 *  Yoctopuce Sarl (hereafter Licensor) grants to you a perpetual
 *  non-exclusive license to use, modify, copy and integrate this
 *  file into your software for the sole purpose of interfacing 
 *  with Yoctopuce products. 
 *
 *  You may reproduce and distribute copies of this file in 
 *  source or object form, as long as the sole purpose of this
 *  code is to interface with Yoctopuce products. You must retain 
 *  this notice in the distributed source file.
 *
 *  You should refer to Yoctopuce General Terms and Conditions
 *  for additional information regarding your rights and 
 *  obligations.
 *
 *  THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT
 *  WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING 
 *  WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, FITNESS 
 *  FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO
 *  EVENT SHALL LICENSOR BE LIABLE FOR ANY INCIDENTAL, SPECIAL,
 *  INDIRECT OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, 
 *  COST OF PROCUREMENT OF SUBSTITUTE GOODS, TECHNOLOGY OR 
 *  SERVICES, ANY CLAIMS BY THIRD PARTIES (INCLUDING BUT NOT 
 *  LIMITED TO ANY DEFENSE THEREOF), ANY CLAIMS FOR INDEMNITY OR
 *  CONTRIBUTION, OR OTHER SIMILAR COSTS, WHETHER ASSERTED ON THE
 *  BASIS OF CONTRACT, TORT (INCLUDING NEGLIGENCE), BREACH OF
 *  WARRANTY, OR OTHERWISE.
 *
 *********************************************************************/

Content of this package:
=======================
build.bat                      Automated build script for Windows
build.sh                       Automated build script for UNIX platforms
FILES.txt                      List of files contained in this archive
RELEASE.txt                    Release notes
Binaries/GNUmakefile           Makefile for UNIX platforms
Binaries/makefile              Makefile for Windows (nmake)
Binaries/make.bat              Batch to start nmake on Windows with right paths
Binaries/windows/              Directory that contains Windows executables
Binaries/osx/                  Directory that contains Max OS X executables
Binaries/linux/32bits/         Directory that contains Linux 32bit executables
Binaries/linux/64bits/         Directory that contains Linux 64bit executables
Binaries/linux/armel/          Directory that contains Linux ARM soft float executables
Binaries/linux/armhf/          Directory that contains Linux ARM hard float executables
Documentation/                 API Reference, in HTML and PDF format
Examples/                      Directory with sample programs in C++
Sources/                       Source code of the high-level library (in C++)
Sources/yapi/                  Source code of the low-level library (in C)
udev_conf/                     Udev rules for linux (see Linux Release Notes)

The archive is shipped with precompiled libraries. If you want to rebuild 
them from source, or to compile the examples, use the following command:

on Windows: build
on UNIX:    ./build.sh

For more details, refer to the documentation specific to each product, which
includes sample code with explanations, and a programming reference manual.
In case of trouble, contact support@yoctopuce.com

Have fun !


Linux Release Notes :
=====================

Libusb 1.0:
----------

In order to compile the library you have to install the version 1.0 of libusb. 
Take care to use version 1.0 and not version 0.1. To install libusb 1.0 on 
Ubuntu, run "sudo apt-get install libusb-1.0-0-dev".


Configure udev access rights:
----------------------------

In order to work properly, the Yoctopuce VirtualHub and library need write
access to all Yoctopuce devices. By default, Linux access rights for USB 
device are read only for all users, except root. If you want to avoid running 
VirtualHub as root, you need to add a new rule to your udev configuration.

To add a new udev rules to your Linux installation, you need to create a text 
file in the directory "/etc/udev/rules.d" following the naming pattern "##-
arbitraryName.rules". Upon startup, udev will process all files in this 
directory with the extension ".rules" according to there alphabetical order. 
For instance, the file "51-first.rules" will be processed before  the file "50-
udev-default.rules". The file "50-udev-default.rules" is actually used to 
implement the default rules of the system. Therefore, to modify the default 
handling behaviour of the system, you have to create a file that start with a 
number lower than 50. Note that to add a rules to your udev configuration you 
have to be root.

In the sub directory udev_conf we have put two examples of rules that you can 
use as reference for your rules.

Example 1: 51-yoctopuce.rules

This rule will add write access to Yoctopuce USB devices for all users. Access 
rights for all other devices will be left unchanged. If this is what you want, 
copy the file "51-yoctopuce_all.rules" to the directory  "/etc/udev/rules.d" 
and restart your system.

    # udev rules to allow write access to all users for Yoctopuce USB devices
    SUBSYSTEM=="usb", ATTR{idVendor}=="24e0", MODE="0666"

Example 2: 51-yoctopuce_group.rules

This rule will allow write access to Yoctopuce USB devices for all users of 
the group "yoctogoup". Access right for all other devices will be left 
unchanged. If this is what you want, you need to copy the file "51-
yoctopuce_all.rules" to the directory  "/etc/udev/rules.d" and restart your 
system.

    # udev rules to allow write access to all users of "yoctogroup" for Yoctopuce USB devices
    SUBSYSTEM=="usb", ATTR{idVendor}=="24e0", MODE="0664",  GROUP="yoctogroup"

