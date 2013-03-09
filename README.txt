I need to put something here, I guess...

Authors
-----
Jesse DeGuire, original creator


Intro
-----
This is the Projector Controller project.  The goal is to provide the DIY projector community with a robust and simple means of controlling basic functionality of their DIY light-beaming creations.  This includes things like monitoring temperature (and shutting the projector down if it gets too hot), controlling fan speed to keep the projector quiet, and turning the lamp on when the user presses the "On" button on the remote or OSD panel.

I once had a DIY projector that did much of this (rather poorly) using relays, attic fan thermostats, and way too much wiring, thus the inspiration for this project.

This project also shows that I'm terrible at coming up with clever names...


The Good Stuff
-----
The Projector Controller is made up of firmware that runs on the microcontroller on the board, software that runs on the PC and talks to the board, and the board hardware itself.

There are actually two firmwares:  the bootloader and the application.  If you plan on building either of those, you'll need the following:
--avr-gcc (Ubuntu's repo has this as 'gcc-avr' for some reason)
--avr-binutils (Ubuntu has this as 'binutils-avr')
--avr-libc
--avrdude
--scons, which in turn requires Python

The SConscript/SConstruct files assume that you are using the avr-gcc toolchain and avrdude as your
ICSP programmer.  You can modify the files if this is not the case.

To modify or run the software, you'll need:
--Python (written in v2.7)
--PySide
--PySerial

To build the software into an executable, you'll need PyInstaller and whatever prerequisites it requires.  I have been able to build a Windows executable under Windows 7 Professional Edition 64-bit without too much trouble.  This is available at /software/win32.

The schematic and board layout are created with KiCAD.  The board is currently manufactured at OSH Park (www.oshpark.com).  The Bill of Materials is available in the repo at /hardware/BoM.ods as an Open Document Spreadsheet.  LibreOffice, OpenOffice, and newer versions of Microsoft Word can open it.


More Info
-----
The Developers Guide located at /docs/devguide.pdf contains much more detailed information about the hardware, firmware, and software implementations.  Tinkerers and curious users should look at that guide and the accompanying source code and schematics.  The document is written in LaTeX and the source is in the /docs/tex directory.


Licensing
-----
The EDA design files (in hardware/pjc-kicad), along with the documents and their sources (in docs/
and docs/tex) are licensed under the Creative Commons Attribution-ShareAlike 3.0 License.  The
software and firmware code (in software/ and firmware/) , as well as their SCons build scripts, are
available under the GPLv3.
