#! /usr/bin/env python

"""
pjcbootloader.py

Contains classes used for interacting with the bootloader on the ATmega devices.  Generally, you
will want to use the PJCBootloader class to do all of the talking to the device.
"""

class PJCBootloader:
    """An interface to the PJC bootloader for ATMega devices.

    A bootloader allows an application to update firmware on the device without requiring an
    external programmer.  This class exposes the various capabilities of the PJC bootloader beyond
    code updates, such as erasing flash and EEPROM and getting a firmware app's checksum.
    """

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
                        end = start + record.dataSize
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
                    break

        return result


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

        self.dataSize = self.rawdata[0]
        self.address = (self.rawdata[1] << 8) | self.rawdata[2]
        self.type = self.rawdata[3]
        self.data = [self.rawdata[4 + i] for i in range(self.dataSize)]
        self.checksum = self.rawdata[4 + self.dataSize]


    def verifyChecksum(self):
        """Return True if the record checksum is correct."""
        return (sum(self.rawdata) & 0xFF) == 0
