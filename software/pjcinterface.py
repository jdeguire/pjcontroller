#! /usr/bin/env python
#
# Copyright 2011-2013 Jesse DeGuire
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
File:   pjcinterface.py
Author: Jesse DeGuire

Contains base class PJCInterface for talking to the PJC device.
"""

import re
import sys

import serial

import pjcexcept


class PJCInterface:
    """A generic synchronous serial interface to the ATMega device used in the PJC board.

    This class provides basic methods for talking to programs running on the microcontroller.
    Communication is based on simple ASCII commands and is normally synchronous.  This implements a
    few commands that should be common to apps or a bootloader running on the device.  Subclasses
    would implement commands specific to the apps or bootloader they are meant to communicate with.

    This class uses PySerial for communication.
    """

    RespString = 0       # leave response as string without parsing
    RespDecimal = 1      # parse as a decimal integer
    RespHex = 2          # parse as a hexidecimal integer
    RespFloat = 3        # parse as a floating-point number
    RespStatus = 4       # parse as a bootloader status code ("!xx")
    RespStrList = 5      # list of strings with each element separated by a CR ('\r')
    RespDecList = 6      # list of decimal integers
    RespHexList = 7      # list of hexidecimal integers
    RespFltList = 8      # list of floating-point numbers

    CommandPrompt = '\r#> '
    StartupString = '---Startup---\r'

    def __init__(self, serialdevice):
        """Create a new PJCInterface object which will communicate over the given serial port.
        """
        self.serial = serialdevice

    def execCommand(self, cmd, resptype=RespString, timeout=1.0):
        """Execute a command and parse the response into a usable form.

        Pass one of the Resp constants in this class to 'restype' to control how the response is
        parsed.  This will raise a subclass of PJCError if a problem occurs.  See the module
        pjcexpect for the classes of exceptions that could be raised.
        """
        self._sendCommand(cmd)
        respstr = self._readSerialResponse(timeout)

        if respstr:
            if PJCInterface.StartupString in respstr:
                desc = 'Device restarted while executing command "' + cmd.partition(' ')[0] + '".'
                raise pjcexcept.DeviceRestartError(cmd, desc)
            elif 'Unknown command\r' in respstr:
                desc = 'Device does not recognize command "' + cmd.partition(' ')[0] + '".'
                raise pjcexcept.UnknownCommandError(cmd, desc)
            else:
                try:
                    result = self._parseResponseString(respstr, resptype)
                except ValueError:
                    desc = 'Command "' + cmd + '" yielded unexpected response.'
                    raise pjcexcept.UnexpectedResponseError(cmd, desc)
        else:
            desc = 'Device did not respond to command "' + cmd.partition(' ')[0] + '".'
            raise pjcexcept.NotRespondingError(cmd, desc)

        return result

    def clearInterface(self):
        """Clear the interface by telling the firmware to discard any command data is has received.
        """
        self._sendCommand('\x1B')   # Esc char
        self._readSerialResponse()

    def isApplication(self):
        """Return True if an application appears to be running on the device or False if a
        bootloader is running.
        """
        return ('Bootloader' not in self.execCommand('v'))

    def getVersion(self):
        """Get the version of the code running on the device or -1 if no version number is given by
        the device.
        """
        result = -1
        match = re.search(r' v(\d+)', self.execCommand('v'))

        if match:
            result = int(match.group(1))

        return result

    def getAppCRC(self):
        """Get the 16-bit CRC of the application currently on the device.
        """
        return self.execCommand('crc', PJCInterface.RespHex) & 0xFFFF

    def doJump(self):
        """Jump from bootloader to application or vice versa.  Returns True if the device started
        the application or False if it started the bootloader.
        """
        try:
            return ('Bootloader' not in self.execCommand('j', timeout=5.0))
        except pjcexcept.DeviceRestartError:
            return self.isApplication()

    def resetDevice(self):
        """Reset device and restart in whatever program it was running previously.  Returns True if
        the device started the application or False if it started the bootloader.

        Note that the bootloader may start up even if the application should have in the event that
        the application could not be started.
        """
        try:
            return ('Bootloader' not in self.execCommand('r', timeout=5.0))
        except pjcexcept.DeviceRestartError:
            return self.isApplication()

    def _sendCommand(self, cmd):
        """Flush out any previous command data and send a new command, adding the proper line
        ending.
        """
        if self.serial.isOpen():
            waiting = self.serial.read(self.serial.inWaiting())

            if PJCInterface.StartupString in waiting:
                desc = 'Device restarted since the last command was issued.'
                raise pjcexcept.DeviceRestartError(cmd, desc)

            self.serial.write(cmd + '\r')
        else:
            raise pjcexcept.SerialPortNotOpenError(cmd, 'No serial port is currently open.')

    def _readSerialResponse(self, timeout=1.0):
        """Read in any data available from the device.

        This reads the serial port until it receives the prompt (#>) or until it times out.  The
        prompt is removed from the response.
        """
        resp = ''

        self.serial.timeout = timeout

        temp = self.serial.read(max(self.serial.inWaiting(), 1))
        resp += temp

        while temp != ''  and  not resp.endswith(PJCInterface.CommandPrompt):
            temp = self.serial.read(max(self.serial.inWaiting(), 1))
            resp += temp

        resp = resp.replace(PJCInterface.CommandPrompt, '')
        return resp

    def _parseResponseString(self, respstr, resptype=RespString):
        """Parse the command response string according to the type given by 'resptype'.

        This will raise a ValueError if the string cannot be parsed because it contains unexpected
        or invalid characters.
        """
        if PJCInterface.RespDecimal == resptype:
            result = int(respstr, 10)
        elif PJCInterface.RespHex == resptype:
            result = int(respstr, 16)
        elif PJCInterface.RespFloat == resptype:
            result = float(respstr)
        elif PJCInterface.RespStatus == resptype:
            result = int(respstr[respstr.index('!')+1:], 16)
        elif PJCInterface.RespStrList == resptype:
            result = respstr.splitlines()
        elif PJCInterface.RespDecList == resptype:
            result = [int(i, 10) for i in respstr.splitlines()]
        elif PJCInterface.RespHexList == resptype:
            result = [int(i, 16) for i in respstr.splitlines()]
        elif PJCInterface.RespFltList == resptype:
            result = [float(i) for i in respstr.splitlines()]
        else:
            result = respstr

        return result
