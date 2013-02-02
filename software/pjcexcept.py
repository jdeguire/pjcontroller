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
File:   pjcexcept.py
Author: Jesse DeGuire

Defines exceptions that can be raised when trying to communicate over the serial port.  
"""

class PJCError(Exception):
    """ Base class for exceptions that can be thrown by the PJC interface classes.
    """
    
    def __init__(self, badcmd, desc):
        """Create a new PJCError object with the command that caused the exception and a
        user-readable description of the problem.

        The 'badcmd' input should be the entire command string, including any arguments passed.
        """
        self.badcmd = badcmd
        self.desc = desc

    def __str__(self):
        return self.desc


class NotRespondingError(PJCError):
    """ Exception raised if the device does not appear to be responding; that is, it never sent back
    a response to a sent command.
    """
    pass


class UnknownCommandError(PJCError):
    """ Exception raised if the device responds with a message stating it does not recognize a
    command.
    """
    pass


class UnexpectedResponseError(PJCError):
    """ Exception raised if the response to a command cannot be parsed properly.
    """
    pass

class DeviceRestartError(PJCError):
    """ Exception raised if the device restarted at some point since the last command.  If the
    command is supposed to restart the device, then this exception will have to be caught and
    handled appropriately.
    """
    pass

class SerialPortNotOpenError(PJCError):
    """ Exception raised if the response to a command cannot be parsed properly.
    """
    pass
