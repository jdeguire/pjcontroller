#! /usr/bin/env python

"""
pjcbootloader.py

Contains classes used for interacting with the bootloader on the ATmega devices.  Generally, you
will want to use the PJCBootloader class to do all of the talking to the device.
"""

import serial
import re
import sys

class BootStatus:
    """Contains constants used by the bootloader to indicate the reason why it is running instead of
    starting up the application.
    """

    BootOK = 0           # bootup OK; we'll never see this if we're in the bootloader
    BootPinSet = 1       # the onboard jumper is set, telling the bootloader to stay there
    RestartByApp = 2     # the application jumped to the bootloader
    NoAppLoaded = 3      # there is no application on board
    BadCRC = 4           # the application's CRC is not what it should be


class CmdResult:
    """Contains constants returned by the PJCBootloader class methods to indicate success or a
    failure.  Positve values are returned by the bootloader and negative values are returned by
    these functions to indicate a communication problem.
    """

    UnexpectedRestart = -2     # bootloader restarted when it shouldn't have
    NotResponding = -1         # bootloader is not talking; communication lost
    CmdOK = 0                  # no error; used to indicate success
    InvalidArgs = 1            # invalid or missing arguments
    PageOutOfBounds = 2        # page out of bounds
    BadChecksum = 3            # expected checksum does not match calculated
    VerifyFailure = 4          # page verify failed


class PJCBootloader:
    """An interface to the PJC bootloader for ATMega devices.

    A bootloader allows an application to update firmware on the device without requiring an
    external programmer.  This class exposes the various capabilities of the PJC bootloader beyond
    code updates, such as erasing flash and EEPROM and getting a firmware app's checksum.  Many of
    these functions will return codes from the CmdResult class in this module.
    """

    SerialBaud = 115200
    SerialTimeout = 1.0
    CommandPrompt = '\r#> '
    StartupString = 'PJC Bootloader'

    def __init__(self, serialdevice):
        """Create a new PJCBootloader object which will communicate over the given serial port.
        """
        self.serial = serialdevice

    def getBootloaderVersion(self):
        """Get the bootloader version loaded on the device or a Not Responding error if that cannot
        be read.
        """
        result = CmdResult.NotResponding

        self._sendCommand('v')
        
        resp = self._readSerialResponse()
        match = re.search(r'Bootloader v\d+', resp)

        if match:
            result = int(match.group(0)[12:])

        return result

    def getAppCRC(self):
        """Get the 16-bit CRC of the application currently on the device or -1 if that cannot be
        read.
        """
        result = -1

        self._sendCommand('crc')

        resp = self._readSerialResponse()

        if resp != '':
            result = int(resp, 16) & 0xFFFF

        return result

    def jumpToApp(self):
        """Jump from bootloader to application.

        Jump from the bootloader by issuing a serial command and parse the response to see if the
        jump was successful.  If the bootloader restarts, then there is probably no app (or a bad
        app) on board and this function will return False.  Otherwise, this will return True.
        """
        result = True

        self._sendCommand('j')
    
        resp = self._readSerialResponse()

        if resp.find(PJCBootloader.StartupString) >= 0:
            result = False
            
        return result

    def restartBootloader(self):
        """Reset device and restart in the bootloader.

        Restart the bootloader by issuing a serial command and parse the the response to see if the
        reset was successful.  If the bootloader restarts, then return True.  Else return False.
        """
        result = False

        self._sendCommand('r')
    
        resp = self._readSerialResponse()

        if resp.find(PJCBootloader.StartupString) >= 0:
            result = True
            
        return result

    def eraseApp(self):
        """Erase the application on board.

        Return True if the application was erased or False otherwise.
        """
        result = False
        
        self._sendCommand('ea yes')   # 'yes' required to confirm erase

        resp = self._readSerialResponse()

        if resp.find('00') >= 0:        # returns '!00' on success
            result = True

        return result

    def eraseEEPROM(self):
        """Erase the device's EEPROM.

        Return True if the EEPROM was erased or False otherwise.
        """
        result = False
        temp = self.serial.timeout

        self.serial.timeout = 6.0       # erasing EEPROM can take a really long time
        
        self._sendCommand('ee yes')   # 'yes' required to confirm erase

        resp = self._readSerialResponse()

        if resp.find('00') >= 0:        # returns '!00' on success
            result = True

        self.serial.timeout = temp

        return result

    def programPage(self, pagenum, pagedata):
        """Program one page of data onto the device's flash.

        Parameters are the page number to program and a single page of data.

        This method returns 0 on success, 1 if the command arguments are malformed, 2 if the page
        number is out of bounds, 3 if the checksum sent to the board as part of the command is
        wrong, 4 if the flash could not be written to correctly, -1 if the bootloader is not
        responding, or -2 if the bootloader restarted unexpectedly.
        """
        result = -1

        checksum = sum(pagedata) & 0xFFFF
        data = ''.join([chr(i) for i in pagedata])

        self._sendCommand('pp ' + hex(pagenum)[2:] + ' ' + hex(checksum)[2:])

        resp = self.serial.read(1)

        if resp.find(':') >= 0:         # ':' signals that it's OK to send data
            self.serial.write(data)
            resp = ''

        resp += self._readSerialResponse()

        if resp.find(PJCBootloader.StartupString) >= 0:
            result = -2
        else:
            match = re.search(r'!0[0-4]', resp)      # valid responses are '!00' - '!04'

            if match:
                result = int(match.group(0)[1:], 16)

        return result

    def writeCRC(self):
        """Write the application CRC to flash.

        This should be called after the entire application has been programmed to flash using
        repeated calls to programPage().  This CRC is used to verify that a valid application is on
        the board at startup.  Returns True if the CRC was written successfully or False otherwise.
        """
        result = False
        
        self._sendCommand('wc')

        resp = self._readSerialResponse()

        if resp.find('00') >= 0:        # returns '!00' on success
            result = True

        return result

    def getPageSize(self):
        """Get the size in bytes of a single page or -1 if that could not be read.

        The result is stored in the calling object for use with the programPage() and
        calculateFileCRC() methods.
        """
        result = -1

        self._sendCommand('ps')
        
        resp = self._readSerialResponse()

        if resp != '':
            result = int(resp, 16) & 0xFFFF

        self.pagesize = result
        return result

    def getMaxPages(self):
        """Get the maximum number of pages an application can occupy or -1 if that could not be
        read.

        The result is stored in the calling object for use with the programPage() and
        calculateFileCRC() methods.
        """
        result = -1

        self._sendCommand('pn')
        
        resp = self._readSerialResponse()

        if resp != '':
            result = int(resp, 16) & 0xFFFF

        self.maxpages = result
        return result

    def getBootStatus(self):
        """Get a value representing the reason why the device is running the bootloader instead of
        the application or -1 if that could not be read.

        Return 1 if the bootloader pin is set, 2 on a watchdog/application reset, 3 if no app is on
        the board, or 4 if the app checksum is bad.  This will never return 0 since that would mean
        that the bootloader will have started the application.
        """
        result = -1

        self._sendCommand('s')
        
        resp = self._readSerialResponse()
        match = re.search(r'(0[0-4])', resp)

        if match:
            result = int(match.group(0)[1:])

        return result        

    def _readSerialResponse(self):
        """Read the response to a command.  

        This reads the serial port until it receives the prompt (#>) or until it times out.  The
        prompt is removed from the response.
        """
        resp = ''

        temp = self.serial.read(max(self.serial.inWaiting(), 1))
        resp += temp
 
        while temp != ''  and  not resp.endswith(PJCBootloader.CommandPrompt):
            temp = self.serial.read(max(self.serial.inWaiting(), 1))
            resp += temp

        resp = resp.replace(PJCBootloader.CommandPrompt, '')
        return resp

    def _sendCommand(self, cmd):
        """Flush out any previous command data and send a new command, adding the proper line
        ending.
        """
        self.serial.flushInput()
        self.serial.write(cmd + '\r')
