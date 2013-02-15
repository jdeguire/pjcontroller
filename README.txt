I need to put something here, I guess...

If you plan on building the firmware, you'll need a few things.
--avr-gcc (Ubuntu's repo has this as 'gcc-avr' for some reason)
--avr-binutils
--avr-libc
--avrdude
--scons, which in turn requires Python

To modify or run the software, you'll need:
--Python (written in v2.7)
--PySide
--PySerial

The SConscript/SConstruct files assume that you are using the avr-gcc toolchain and avrdude as your
ICSP programmer.  You can modify the files if this is not the case.

Schematics are created with KiCAD.

The EDA design files (in hardware/pjc-kicad), along with the documents and their sources (in docs/
and docs/tex) are licensed under the Creative Commons Attribution-ShareAlike 3.0 License.  The
software and firmware code (in software/ and firmware/) , as well as their SCons build scripts, are
available under the GPLv3.
