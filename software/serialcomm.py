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
File:   commthread.py
Author: Jesse DeGuire

Contains a class for talking to the devices over a serial port and for the commands it will accept.
"""

import serial

from PySide import QtCore
from PySide.QtCore import QObject

from pjcbootloader import PJCBootloader
from pjcapplication import PJCApplication
from flashimage import FlashImage
from connmanager import ConnectionManager
import pjcexcept

# This package is available in PySerial 2.6, but at this time (Feb. 1, 2013) still comes with
# PySerial 2.5, so make sure we can go on without it.
try:
    from serial.tools.list_ports import comports
except ImportError:
    comports = None


class SerialComm(QObject):
    """Class for serial communications.

    This class communicates with the board over a serial connection.  Commands are retrieved via QT
    slots and the results are transmitted using QT signals.
    """

    # new signals have to be declared out here, something the docs aren't very explicit about
    devicestarted = QtCore.Signal(bool)
    serialclosed = QtCore.Signal()
    serialenumerated = QtCore.Signal(list)
    updateprogressed = QtCore.Signal(int)
    updatecompleted = QtCore.Signal(bool)
    newlogmessage = QtCore.Signal(str)      # used to print messages to the UI
    lampstateupdated = QtCore.Signal(bool)
    targettempupdated = QtCore.Signal(float)
    overtempupdated = QtCore.Signal(float)
    fanoffupdated = QtCore.Signal(float)
    mindutycycleupdated = QtCore.Signal(float)
    monitorrefreshed = QtCore.Signal(tuple)

    def __init__(self, connmgr):
        QObject.__init__(self)

        self.serialdev = serial.Serial(None, 115200, timeout=1.0)
        self.pjcboot = PJCBootloader(self.serialdev)
        self.pjcapp = PJCApplication(self.serialdev)

        # set up connections
        connmgr.addSignal(self.devicestarted, 'DeviceStarted')
        connmgr.addSignal(self.serialclosed, 'SerialClosed')
        connmgr.addSignal(self.serialenumerated, 'SerialEnumerated')
        connmgr.addSignal(self.newlogmessage, 'WriteToLog')
        connmgr.addSlot(self.enumerateSerialPorts, 'EnumerateSerial')
        connmgr.addSlot(self.openSerialPort, 'OpenSerial')

        connmgr.addSignal(self.updateprogressed, 'UpdateProgressed')
        connmgr.addSignal(self.updatecompleted, 'UpdateCompleted')
        connmgr.addSlot(self.doFirmwareUpdate, 'StartUpdate')

        connmgr.addSignal(self.lampstateupdated, 'LampEnableChanged')
        connmgr.addSignal(self.targettempupdated, 'TargetTempChanged')
        connmgr.addSignal(self.overtempupdated, 'OvertempChanged')
        connmgr.addSignal(self.fanoffupdated, 'FanOffTempChanged')
        connmgr.addSignal(self.mindutycycleupdated, 'MinDutyCycleChanged')
        connmgr.addSlot(self.setLampState, 'SetLampEnable')
        connmgr.addSlot(self.setTargetTemp, 'SetTargetTemp')
        connmgr.addSlot(self.setOvertempLimit, 'SetOvertemp')
        connmgr.addSlot(self.setFanOffTemp, 'SetFanOffTemp')
        connmgr.addSlot(self.setMinDutyCycle, 'SetMinDutyCycle')
        connmgr.addSlot(self.refreshAppSettings, 'RefreshSettings')
        connmgr.addSlot(self.saveAppSettings, 'SaveSettings')

        connmgr.addSignal(self.monitorrefreshed, 'MonitorRefreshed')
        connmgr.addSlot(self.refreshMonitorData, 'RefreshMonitor')
        connmgr.addSlot(self.printBootStatus, 'PrintBootStatus')

    def __del__(self):
        if self.serialdev.isOpen():
            self.serialdev.close()

    def _print(self, text):
        self.newlogmessage.emit(text)

    def _handlesPJCExceptions(func):
        """A decorator allowing the decorated functions to catch and handle exceptions thrown from
        the PJCInterface classes in a consistent manner.
        """
        def wrapper(self, *args, **kwargs):
            try:
                func(self, *args, **kwargs)
            except pjcexcept.NotRespondingError:
                self._print('The device is not responding; closing serial port.')
                self.serialdev.close()
                self.serialclosed.emit()
            except pjcexcept.UnknownCommandError:
                self._print('The device did not recognize an issued command.')
            except pjcexcept.UnexpectedResponseError:
                self._print('The device responded incorrectly to a command; closing serial port.')
                self.serialdev.close()
                self.serialclosed.emit()
            except pjcexcept.DeviceRestartError:
                self._print('The device restarted unexpectedly.')
                self.devicestarted.emit(self.pjcboot.isApplication())
            except (pjcexcept.SerialPortNotOpenError, serial.portNotOpenError):
                self._print('No serial port is open.')

        return wrapper

    @QtCore.Slot()
    @_handlesPJCExceptions
    def enumerateSerialPorts(self):
        fallback = True

        if comports:
            try:
                ports = [name for name, unused_desc, unused_hwid in sorted(comports())]
                fallback = False
            except:
                fallback = True

        if fallback:
            # PySerial 2.6 has a bug in which comports() throws an exception with USB devices, so
            # handle that and the case of having an older version.  This should always work, but
            # will miss some ports (like USB ones).
            ports = []

            for i in range(256):
                try:
                    s = serial.Serial(i)
                    ports.append(s.name)
                    s.close()
                except serial.SerialException:
                    pass

        self.serialenumerated.emit(ports)

    @QtCore.Slot(str)
    @_handlesPJCExceptions
    def openSerialPort(self, serialpath):
        try:
            if serialpath != self.serialdev.port  or  not self.serialdev.isOpen():
                if self.serialdev.isOpen():
                    self.serialdev.close()
                self.serialdev.port = serialpath
                self.serialdev.open()
                self.devicestarted.emit(self.pjcboot.isApplication())
        except serial.SerialException:
            self._print('Could not open serial port at ' + serialpath + '.')

    @QtCore.Slot(str)
    @_handlesPJCExceptions
    def doFirmwareUpdate(self, hexfile):
        result = True

        if self.pjcboot.isApplication():
            if self.pjcboot.doJump():
                self._print('Failed to jump to bootloader.')
                result = False
            else:
                self.devicestarted.emit(False)

        flashmem = FlashImage(self.pjcboot.getMaxPages(), self.pjcboot.getPageSize())

        if result:
            if flashmem.buildImageFromFile(hexfile):
                self._print('Erasing old app...')
                self.pjcboot.eraseApp()

                self._print('Loading new app...')

                for i in range(flashmem.getUsedAppPages()):
                    if self.pjcboot.loadPageData(flashmem.getSinglePage(i)):
                        if self.pjcboot.programPage(i):
                            self.updateprogressed.emit(i * 100 // flashmem.getUsedAppPages())
                        else:
                            self._print('Failed to program page ' + str(i) + '.')
                            result = False
                            break;
                    else:
                        result = False
                        self._print('Failed to load page data ' + str(i) + '.')
                        break;

                if result:
                    self.pjcboot.writeCRC()
                    if self.pjcboot.doJump():
                        self.devicestarted.emit(True)
                    self._print('Update complete!')
            else:
                self._print('File parse failed.')

        self.updatecompleted.emit(result)

    @QtCore.Slot(bool)
    @_handlesPJCExceptions
    def setLampState(self, state):
        self.lampstateupdated.emit(self.pjcapp.enableLamp(state))

    @QtCore.Slot(float)
    @_handlesPJCExceptions
    def setTargetTemp(self, temp):
        self.targettempupdated.emit(self.pjcapp.setTargetTemperature(temp))

    @QtCore.Slot(float)
    @_handlesPJCExceptions
    def setOvertempLimit(self, temp):
        self.overtempupdated.emit(self.pjcapp.setOvertempLimit(temp))

    @QtCore.Slot(float)
    @_handlesPJCExceptions
    def setFanOffTemp(self, temp):
        self.fanoffupdated.emit(self.pjcapp.setFanOffPoint(temp))

    @QtCore.Slot(float)
    @_handlesPJCExceptions
    def setMinDutyCycle(self, mindc):
        self.mindutycycleupdated.emit(self.pjcapp.setMinDutyCycle(mindc))

    @QtCore.Slot()
    @_handlesPJCExceptions
    def refreshAppSettings(self):
        self.lampstateupdated.emit(self.pjcapp.isLampEnabled())
        self.targettempupdated.emit(self.pjcapp.getTargetTemperature())
        self.overtempupdated.emit(self.pjcapp.getOvertempLimit())
        self.fanoffupdated.emit(self.pjcapp.getFanOffPoint())
        self.mindutycycleupdated.emit(self.pjcapp.getMinDutyCycle())

    @QtCore.Slot()
    @_handlesPJCExceptions
    def saveAppSettings(self):
        if self.pjcapp.saveSettingsToEEPROM():
            self._print('Settings saved.')
        else:
            self._print('Could not save settings.')

    @QtCore.Slot()
    @_handlesPJCExceptions
    def refreshMonitorData(self):
        self.monitorrefreshed.emit((self.pjcapp.readADCs(), 
                                    self.pjcapp.getCurrentDutyCycle(), 
                                    self.pjcapp.isLampEnabled(), 
                                    self.pjcapp.getMostRecentError(True)))

    @QtCore.Slot()
    @_handlesPJCExceptions
    def printBootStatus(self):
        status = self.pjcboot.getBootStatus()

        if status == PJCBootloader.BootStatusOK:
            self._print('The application is running OK.')
        elif status == PJCBootloader.BootStatusPinSet:
            self._print('The bootloader startup jumper is set.')
        elif status == PJCBootloader.BootStatusRestart:
            self._print('The application jumped to the bootloader.')
        elif status == PJCBootloader.BootStatusNoApp:
            self._print('The device does not have an application.')
        elif status == PJCBootloader.BootStatusBadCRC:
            self._print('The application on board is corrupt.')
        else:
            self._print('The device is in the bootloader for an unknown reason.')
