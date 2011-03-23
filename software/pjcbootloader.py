#! /usr/bin/env python

"""
pjcbootloader.py

Contains classes used for interacting with the bootloader on the ATmega devices.  Generally, you
will want to use the PJCBootloader class to do all of the talking to the device.
"""

import serial
import re


class PJCBootloader:
    """An interface to the PJC bootloader for ATMega devices.

    A bootloader allows an application to update firmware on the device without requiring an
    external programmer.  This class exposes the various capabilities of the PJC bootloader beyond
    code updates, such as erasing flash and EEPROM and getting a firmware app's checksum.
    """

    SerialBaud = 115200
    SerialTimeout = 1.0
    CommandPrompt = '\r#> '
    StartupString = 'PJC Bootloader'

    def __init__(self, serialpath):
        """Create a new PJCBootloader object which will communicate over the given serial port.

        Raises a SerialException if the serial port can not be opened.
        """
        self.serial = serial.Serial(serialpath, PJCBootloader.SerialBaud, 
                                    timeout = PJCBootloader.SerialTimeout)
        self.pagesize = -1
        self.maxpages = -1
        self.flashimage = []

    def parseFile(self, filepath):
        """Parse Intel Hex file into a binary image.
        
        Returns True if the file was parsed successfully and False if it wasn't (record checksum is
        incorrect).  Raises an IOError if the file could not be opened.
        """
        self.flashimage = []
        
        result = False
        extAddr = 0

        with open(filepath, 'r') as hexfile:
            for line in hexfile:
                record = IntelHexRecord(line)
                
                if record.verifyChecksum():
                    if record.type == IntelHexRecord.DataRec:
                        start = extAddr + record.address
                        end = start + record.datasize
                        size = len(self.flashimage)

                        # fill in gaps with 0xFF; the value of erased flash
                        if size < end:
                            self.flashimage.extend([0xFF] * (end + 1 - size))

                        self.flashimage[start:end] = record.data
                    elif record.type == IntelHexRecord.EOFRec:
                        break
                    elif record.type == IntelHexRecord.SegAddrRec:
                        extAddr = (record.data[0] << 12) | (record.data[1] << 4)
                    elif record.type == IntelHexRecord.ExtAddrRec:
                        extAddr = (record.data[0] << 24) | (record.data[1] << 16)

                    result = True
                else:
                    self.flashimage = []
                    result = False
                    break

        return result

    def calculateFileCRC(self):
        """Calculate the 16-bit CRC of the binary data found with the parseFile() method.

        This requires that the parseFile() method was successfully run and that the object variables
        'pagesize' and 'maxpages' are set.  Use the methods getPageSize() and getMaxPages(),
        respectively, to do this.  Otherwise, this method will return -1.

        The algorithm used here was adapted from example C code provided by the AVR-libc manual.
        See the _crc_ccitt_update() function description at
        http://www.nongnu.org/avr-libc/user-manual/group__util__crc.html.
        """
        crc = 0xFFFF

        if self.pagesize > 0  and  self.maxpages > 0  and  self.flashimage != []:
            for i in range(self.pagesize * self.maxpages):
                if i < len(self.flashimage):
                    by = self.flashimage[i] & 0xFF
                else:
                    by = 0xFF
                
                    by = by ^ (crc & 0xFF)
                    by = by ^ (by << 4)

                    crc = ((((by << 8) & 0xFFFF) | ((crc >> 8) & 0xFF)) ^ ((by >> 4) & 0xFF) ^ 
                           ((by << 3) & 0xFFFF))
        else:
            crc = -1

        return crc

    def getFileNumPages(self):
        """Determine the number of pages used by the binary data found with the parseFile() method.

        This requires that the parseFile() method was successfully run and that the object variable
        'pagesize' is set.  Use the getPageSize() method to do this.  Otherwise, this returns -1.
        """
        result = -1

        if self.flashimage != [] and self.pagesize > 0:
            result = 0

            for i in range(len(self.flashimage) - 1, 0, -1):
                if self.flashimage[i] != 0xFF:
                    result = (i + self.pagesize) / self.pagesize
                    break

        return result    

    def getBootloaderVersion(self):
        """Get the bootloader version loaded on the device or -1 if that could not be read.
        """
        result = -1

        self._flushInput()
        self.serial.write('v\r')
        
        resp = self._readSerialResponse()
        match = re.search(r'Bootloader v\d+', resp)

        if match:
            result = int(match.group(0)[1:])

        return result

    def getAppCRC(self):
        """Get the 16-bit CRC of the application currently on the device or -1 if that cannot be
        read.
        """
        result = -1

        self._flushInput()
        self.serial.write('crc\r')

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

        self._flushInput()
        self.serial.write('j\r')
    
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

        self._flushInput()
        self.serial.write('r\r')
    
        resp = self._readSerialResponse()

        if resp.find(PJCBootloader.StartupString) >= 0:
            result = True
            
        return result

    def eraseApp(self):
        """Erase the application on board.

        Return True if the application was erased or False otherwise.
        """
        result = False
        
        self._flushInput()
        self.serial.write('ea yes\r')   # 'yes' required to confirm erase

        resp = self._readSerialResponse()

        if resp.find('80') >= 0:        # returns '!80' on success
            result = True

        return result

    def eraseEEPROM(self):
        """Erase the device's EEPROM.

        Return True if the EEPROM was erased or False otherwise.
        """
        result = False
        temp = self.serial.timeout

        self.serial.timeout = 6.0       # erasing EEPROM can take a really long time
        
        self._flushInput()
        self.serial.write('ee yes\r')   # 'yes' required to confirm erase

        resp = self._readSerialResponse()

        if resp.find('80') >= 0:        # returns '!80' on success
            result = True

        self.serial.timeout = temp

        return result

    def programPage(self, pagenum):
        """Program one page of 'size' bytes onto the device's flash.

        The calling object's 'pagesize' field must be set properly before calling this method.  Use
        the getPageSize() method to do this. 'pagenum' should be less than the result of
        getMaxPages().  The parseFile() method must have been called successfully before this
        method.

        This method returns 0 on success, 1 if the command arguments are malformed, 2 if the page
        number is out of bounds, 3 if the checksum sent to the board as part of the command is
        wrong, 4 if the flash could not be written to correctly, -1 if the bootloader is not
        responding, or -2 if the bootloader restarted unexpectedly.
        """
        result = -1

        start = pagenum * self.pagesize
        checksum = sum(self.flashimage[start:start + self.pagesize]) & 0xFFFF
        data = ''.join([chr(i) for i in self.flashimage[start:start + self.pagesize]])

        # ensure that data is always one page long by padding it with 0xFFs
        if(len(data) < self.pagesize):
            fmt = '{0:\xFF<' + str(self.pagesize) + '}'
            data = fmt.format(data)

        self._flushInput()
        self.serial.write('pp ' + hex(pagenum)[2:] + ' ' + hex(checksum)[2:] + '\r')

        resp = self.serial.read(1)

        if resp.find(':') >= 0:         # ':' signals that it's OK to send data
            self.serial.write(data)
            resp = ''

        resp += self._readSerialResponse()

        if resp.find(PJCBootloader.StartupString) >= 0:
            result = -2
        else:
            match = re.search(r'!8[0-4]', resp)      # valid responses are '!80' - '!84'

            if match:
                result = int(match.group(0)[1:], 16) & 0x7F

        return result

    def writeCRC(self):
        """Write the application CRC to flash.

        This should be called after the entire application has been programmed to flash using
        repeated calls to programPage().  This CRC is used to verify that a valid application is on
        the board at startup.  Returns True if the CRC was written successfully or False otherwise.
        """
        result = False
        
        self._flushInput()
        self.serial.write('wc\r')

        resp = self._readSerialResponse()

        if resp.find('80') >= 0:        # returns '!80' on success
            result = True

        return result

    def getPageSize(self):
        """Get the size in bytes of a single page or -1 if that could not be read.

        The result is stored in the calling object for use with the programPage() and
        calculateFileCRC() methods.
        """
        result = -1

        self._flushInput()
        self.serial.write('ps\r')
        
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

        self._flushInput()
        self.serial.write('pn\r')
        
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

        self._flushInput()
        self.serial.write('s\r')
        
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

    def _flushInput(self):
        """Discard any data waiting in the serial port's input buffer.
        """
        self.serial.flushInput()


class IntelHexRecord:
    """A class representing one record within an Intel Hex file.

    Intel Hex files are text-based, with each line being a type of record.  Each record starts with
    a ':' and ends with a checksum byte.  There are several different types of records which can
    contain program data or address info.  This class converts the text record into a data structure
    usable in code.  For more info, see Wikipedia here:  http://en.wikipedia.org/wiki/Intel_hex.
    """

    DataRec = 0               # contains program data
    EOFRec = 1                # last record in file to signify end
    SegAddrRec = 2            # upper four bits of 20-bit address
    StartAddrRec = 3          # used for x86; not applicable here
    ExtAddrRec = 4            # upper word of 32-bit address
    ExtStartAddrRec = 5       # used for x86; not applicable here

    def __init__(self, line):
        """Build record from a single line of the Intel Hex file."""
        line = line[1:].rstrip()    # remove starting ':' and line ending

        # convert text into bytes
        self.rawdata = [int(line[idx:idx+2], 16) for idx in range(0, len(line), 2)]

        self.datasize = self.rawdata[0]
        self.address = (self.rawdata[1] << 8) | self.rawdata[2]
        self.type = self.rawdata[3]
        self.data = [self.rawdata[4 + i] for i in range(self.datasize)]
        self.checksum = self.rawdata[4 + self.datasize]

    def verifyChecksum(self):
        """Return True if the record checksum is correct."""
        return (sum(self.rawdata) & 0xFF) == 0
