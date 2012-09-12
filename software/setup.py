#! /usr/bin/env python

"""
setup.py

The configuration file for Py2exe, which I got from the tutorial on the Py2exe website.  To use
this, run "python setup.py py2exe" from the command line in this directory.  You may need to add the
Python install directory to your PATH first.
"""

import sys

from distutils.core import setup
import py2exe

# Apparently Py2exe will complain about this DLL, so exclude it.  The program will still run as long
# as the VC2008 x86 Redistributable pack is installed.  Found the solution on Stack Overflow.
setup(options={"py2exe": {"dll_excludes": ["MSVCP90.dll"]}}, 
      windows=['main.py'])
