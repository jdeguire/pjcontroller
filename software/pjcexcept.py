#! /usr/bin/env python

"""
pjcexcept.py

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

