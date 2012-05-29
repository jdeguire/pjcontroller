#! /usr/bin/env python

"""
flashimage.py

Contains classes used to build a binary image of flash memory from an Intel Hex file.
"""


class FlashImage:
    """A class containing a binary image of the contents of AVR flash memory.

    Flash memory on the AVR devices is organized into pages, some of which are allocated to the
    bootloader.  For example, the ATMega328p has 256 pages of 128 bytes each, giving 32KB of flash.
    However, 4KB of that is reserved for the bootloader, so there are 224 pages available to the
    application.  This class assumes that the application starts at address 0.
    """

    def __init__(self, app_pages, page_size):
        """Create a new blank flash image.  Blank flash contains all 1 bits.
        """
        self.app_pages = app_pages
        self.page_size = page_size
        self.clearImage()

    def clearImage(self):
        """Clear the image so that it is blank.  Blank flash memory contains all 1 bits.
        """
        self.used_pages = 0
        self.image = [0xFF] * (self.app_pages * self.page_size)

    def getTotalAppPages(self):
        """Return the total number of flash pages available for application use.
        """
        return self.app_pages

    def getPageSize(self):
        """Return the size of a single flash page.
        """
        return self.page_size

    def getUsedAppPages(self):
        """Return the number of pages containing application data.
        """
        return self.used_pages

    def getSinglePage(self, pagenum):
        """Return a single page of data or an empty list if the given page number is invalid.
        """
        if pagenum < self.app_pages:
            start = self.page_size * pagenum
            end = start + self.page_size

            return self.image[start:end]
        else:
            return []

    def buildImageFromFile(self, filepath):
        """Build the binary image by parsing an Intel Hex file.
        
        Returns True if the file was parsed successfully and False if the file is bad.  This is
        either because a record checksum was bad or because the file addresses an invalid location.
        Raises an IOError if the file could not be opened.
        """
        result = False
        extAddr = 0
        self.clearImage()

        with open(filepath, 'r') as hexfile:
            for line in hexfile:
                record = IntelHexRecord(line)
                
                if record.verifyChecksum():
                    if record.type == IntelHexRecord.DataRec:
                        start = extAddr + record.address
                        end = start + record.datasize

                        # we're outside of flash space, bail out
                        if end > len(self.image):
                            self.clearImage()
                            result = False
                            break

                        self.image[start:end] = record.data
                        current_page = end / self.page_size

                        if current_page >= self.used_pages:
                            self.used_pages = current_page + 1
                    elif record.type == IntelHexRecord.EOFRec:
                        break
                    elif record.type == IntelHexRecord.SegAddrRec:
                        extAddr = (record.data[0] << 12) | (record.data[1] << 4)
                    elif record.type == IntelHexRecord.ExtAddrRec:
                        extAddr = (record.data[0] << 24) | (record.data[1] << 16)

                    result = True
                else:
                    self.clearImage()
                    result = False
                    break

        return result

    def calculateCRC(self):
        """Calculate the 16-bit CRC of the data in the flash image.

        This calculates the CRC of whatever is in the image, even if the image is blank.  The result
        is not cached, so the caller may want to save it if speed is a high priority.

        The algorithm used here was adapted from example C code provided by the AVR-libc manual.
        See the _crc_ccitt_update() function description at
        http://www.nongnu.org/avr-libc/user-manual/group__util__crc.html.
        """
        crc = 0xFFFF

        for by in self.image:
            by ^= (crc & 0xFF)
            by ^= (by << 4) & 0xFF

            crc = ((by << 8) | ((crc >> 8) & 0xFF)) ^ ((by >> 4) & 0xFF) ^ (by << 3)

        return crc


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
        """Build record from a single line of the Intel Hex file.
        """
        line = line[1:].rstrip()    # remove starting ':' and line ending

        # convert text into bytes
        rawdata = [int(line[idx:idx+2], 16) for idx in range(0, len(line), 2)]

        self.datasize = rawdata[0]
        self.address = (rawdata[1] << 8) | rawdata[2]
        self.type = rawdata[3]
        self.data = [rawdata[4 + i] for i in range(self.datasize)]
        self.checksum = rawdata[4 + self.datasize]

    def verifyChecksum(self):
        """Return True if the record checksum is correct.
        """
        return (0 == (sum(self.data) + 
                       self.datasize + 
                       (self.address & 0xFF) + ((self.address >> 8) & 0xFF) +
                       self.type + 
                       self.checksum) & 0xFF)
