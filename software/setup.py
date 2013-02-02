#! /usr/bin/env python
#
# Copyright © 2011-2013 Jesse DeGuire
#
# This file is part of Projector Controller.
#
# Projector Controller is free software: you can redistribute it and/or 
# modify it under the terms of the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Projector Controller is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of 
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
# General Public License for more details.
#
# You should have received a copy of the GNU General Public License along
# with Projector Controller.  If not, see <http://www.gnu.org/licenses/>.

"""
File:   setup.py
Author: Jesse DeGuire

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
