#! /usr/bin/env python

"""
commthread.py

Contains a class for the serial communication thread and for the commands it will accept.
"""

import time
import serial
import pjcbootloader
import flashimage
from PySide import QtCore
from PySide.QtCore import *

class CommThread(QThread):
    """Class for the serial communications thread.

    This class communicates with the board over a serial connection in its own thread.  Commands are
    retrieved via QT slots and the results are transmitted using QT signals.
    """

    # new signals have to be declared out here, something the docs aren't very explicit about
    serialenumerated = QtCore.Signal(list)
    updateprogressed = QtCore.Signal(int)
    updatecompleted = QtCore.Signal(bool)
    newtextmessage = QtCore.Signal(str)      # used to print messages to the UI

    def __init__(self):
        QThread.__init__(self)

        self.serialdev = serial.Serial(None, 115200, timeout=1.0)
        self.pjcboot = pjcbootloader.PJCBootloader(self.serialdev)

    def quit(self):
        self.serialdev.close()
        QThread.quit(self)

    def _print(self, text):
        self.newtextmessage.emit(text)

    @QtCore.Slot()
    def enumerateSerialPorts(self):
        l = []

        for i in range(256):
           try:
                s = serial.Serial(i)
                l.append(s.name)
                s.close()
           except serial.SerialException:
                pass

        self.serialenumerated.emit(l)

    @QtCore.Slot(str)
    def openSerialPort(self, serialpath):
        if serialpath != self.serialdev.port:
            if self.serialdev.isOpen():
                self.serialdev.close()
            self.serialdev.port = serialpath
            self.serialdev.open()

    @QtCore.Slot(str)
    def doFirmwareUpdate(self, hexfile):
        result = True

        # not final, doesn't handle exceptions and doesn't jump from app to bootloader
        if self.pjcboot.isApplication():
            if self.pjcboot.doJump():
                self._print('Failed to jump to bootloader')
            else:
                self._print('Jumped to bootloader')

        flashmem = flashimage.FlashImage(224, 128)      # for ATMega328p

        if flashmem.buildImageFromFile(hexfile):
            self._print('File CRC: ' + hex(flashmem.calculateCRC()))
            self._print('File pages: ' + str(flashmem.getUsedAppPages()))

            self._print('Erasing old app...')
            self.pjcboot.eraseApp()

            self._print('Loading new app...')

            for i in range(flashmem.getUsedAppPages()):
                if self.pjcboot.loadPageData(flashmem.getSinglePage(i)):
                    if self.pjcboot.programPage(i):
                        self.updateprogressed.emit(i * 100 // flashmem.getUsedAppPages())
                    else:
                        self._print('Failed to program page ' + str(i))
                        result = False
                        break;
                else:
                    result = False
                    self._print('Failed to load page data ' + str(i))
                    break;

            if result:
                self.pjcboot.writeCRC()
                self._print('Update complete!')
                self.pjcboot.doJump()
        else:
            self._print('File parse failed')

        self.updatecompleted.emit(result)
