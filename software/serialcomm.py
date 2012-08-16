#! /usr/bin/env python

"""
commthread.py

Contains a class for talking to the devices over a serial port and for the commands it will accept.
"""

import serial
import pjcbootloader
import flashimage
import connmanager
from PySide import QtCore
from PySide.QtCore import QObject

class SerialComm(QObject):
    """Class for serial communications.

    This class communicates with the board over a serial connection.  Commands are retrieved via QT
    slots and the results are transmitted using QT signals.
    """

    # new signals have to be declared out here, something the docs aren't very explicit about
    serialenumerated = QtCore.Signal(list)
    updateprogressed = QtCore.Signal(int)
    updatecompleted = QtCore.Signal(bool)
    newtextmessage = QtCore.Signal(str)      # used to print messages to the UI

    def __init__(self, connmgr):
        QObject.__init__(self)

        self.serialdev = serial.Serial(None, 115200, timeout=1.0)
        self.pjcboot = pjcbootloader.PJCBootloader(self.serialdev)

        # set up connections
        connmgr.addSignal(self.serialenumerated, 'SerialEnumerated')
        connmgr.addSignal(self.updateprogressed, 'UpdateProgressed')
        connmgr.addSignal(self.updatecompleted, 'UpdateCompleted')
        connmgr.addSignal(self.newtextmessage, 'WriteToLog')
        connmgr.addSlot(self.enumerateSerialPorts, 'EnumerateSerial')
        connmgr.addSlot(self.openSerialPort, 'OpenSerial')
        connmgr.addSlot(self.doFirmwareUpdate, 'StartUpdate')

    def __del__(self):
        if self.serialdev.isOpen():
            self.serialdev.close()

    def _print(self, text):
        self.newtextmessage.emit(text)

    def _handlesExceptions(func):
        """A decorator allowing the decorated functions to catch and handle exceptions thrown from
        the PJCInterface classes in a consistent manner.
        """
        def wrapper(self, *args, **kwargs):
            try:
                func(self, *args, **kwargs)
            except Exception as e:
                self._print(str(e))

        return wrapper

    @QtCore.Slot()
    @_handlesExceptions
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
    @_handlesExceptions
    def openSerialPort(self, serialpath):
        if serialpath != self.serialdev.port:
            if self.serialdev.isOpen():
                self.serialdev.close()
            self.serialdev.port = serialpath
            self.serialdev.open()

    @QtCore.Slot(str)
    @_handlesExceptions
    def doFirmwareUpdate(self, hexfile):
        result = True

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
