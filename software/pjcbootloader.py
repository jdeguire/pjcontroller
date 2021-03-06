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
File:   pjcbootloader.py
Author: Jesse DeGuire

Contains a class that implements commands specific to the PJC Bootloader.  As a child of the
PJCInterface class, this is used to communicate to the ATMega device over the serial port.
"""

import re

from pjcinterface import PJCInterface


class PJCBootloader(PJCInterface):
    """An interface to the PJC bootloader for ATMega devices.

    A bootloader allows an application to update firmware on the device without requiring an
    external programmer.  This class exposes the various capabilities of the PJC bootloader beyond
    code updates, such as erasing flash and EEPROM and getting a firmware app's checksum.  Many of
    these functions will return codes from the CmdResult class in this module.
    """

    BootStatusOK = 0          # booted to application OK
    BootStatusPinSet = 1      # bootloader startup pin on the board is set
    BootStatusRestart = 2     # the firmware app jumped to the bootloader
    BootStatusNoApp = 3       # no application is on the board
    BootStatusBadCRC = 4      # the application is corrupt

    def eraseApp(self):
        """Erase the application on board and return True if successful or False otherwise.
        """
        return self.execCommand('ea yes', PJCInterface.RespStatus, 5.0) == 0

    def eraseEEPROM(self):
        """Erase the device's EEPROM and return True if successful or False otherwise.
        """
        return self.execCommand('ee yes', PJCInterface.RespStatus, 5.0) == 0

    def loadPageData(self, pagedata):
        """Load a page of data into the device for writing.  Return True on success or False
        otherwise.
        """
        result = 0
        offset = 0
        d = pagedata[0:16]

        while d:
            checksum = sum(d) & 0xFFFF
            dstr = ''.join(['{:02x}'.format(i) for i in d])
            cmd = 'pd {:x} {:x}'.format(offset, checksum) + ' ' + dstr

            result = self.execCommand(cmd, PJCInterface.RespStatus)

            if result != 0:
                break
            else:
                offset += 16
                d = pagedata[offset:offset+16]

        return result == 0

    def programPage(self, pagenum):
        """Program the data loaded with loadPageData() onto the device's flash.

        The parameter is the page number to program.  Returns True on success or False otherwise.
        """
        cmd = 'pp {:x}'.format(pagenum)
        return self.execCommand(cmd, PJCInterface.RespStatus) == 0

    def writeCRC(self):
        """Write the application CRC to flash and return True on success or False otherwise.

        This should be called after the entire application has been programmed to flash using
        repeated calls to programPage().  This CRC is used to verify that a valid application is on
        the board at startup.  Returns True if the CRC was written successfully or False otherwise.
        """
        return self.execCommand('wc', PJCInterface.RespStatus) == 0

    def getPageSize(self):
        """Get the size in bytes of a single page.
        """
        return self.execCommand('ps', PJCInterface.RespHex)

    def getMaxPages(self):
        """Get the maximum number of pages an application can occupy.
        """
        return self.execCommand('pn', PJCInterface.RespHex)

    def getBootStatus(self):
        """Get a value representing the reason why the device is running the bootloader instead of
        the application or -1 if the bootloader did not provide that info.

        Returns one of the BootStatus values indicating the reason the device is in the bootloader
        or 0 if the device is running the application.
        """
        result = -1

        if self.isApplication():
            result = PJCBootloader.BootStatusOK
        else:
            match = re.search(r'(0[0-4])', self.execCommand('s'))
            
            if match:
                result = int(match.group(0)[1:])

        return result
