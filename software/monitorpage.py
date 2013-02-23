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
File:   monitorpage.py
Author: Jesse DeGuire

Contains the MonitorPage class.
"""

import math

from PySide import QtCore
from PySide.QtCore import *
from PySide.QtGui import *

from connmanager import ConnectionManager

# constants for the Steinhart-Hart equation for the board's 10k thermistor
THM_A = 0.0033540164
THM_B = 0.0002565236
THM_C = 2.605970e-6
THM_D = 6.329261e-8

# application error codes
errorNone = 0                 # no error
errorOvertemp = 0x80          # thermistor temp over limit
errorThmShort = 0x81          # thermistor input shorted
errorThmOpen = 0x82           # thermistor input open (no thermistor connected)


class MonitorPage(QDialog):
    """This page displays data from the firmware, such as ADC values and the fan speed.
    """

    lampstateupdate = QtCore.Signal(bool)     # update others on lamp state
    newlogmessage = QtCore.Signal(str)        # used to print to the log
    printbootstatus = QtCore.Signal()         # print bootloader status to log

    def __init__(self, connmgr):
        QDialog.__init__(self)

        degreesC = unichr(176).encode("latin-1") + 'C'

        self.timer = QTimer(self)
        self.timer.setInterval(1000)    # update every second
        self.timer.setSingleShot(False)

        # widgets in the dialog box
        self.sensortempspin = QDoubleSpinBox()
        self.sensortempspin.setReadOnly(True)
        self.sensortempspin.setSuffix(degreesC)
        self.sensortempspin.setRange(-999, 999)   # spinboxes are sized based on range
        self.sensortempspin.setButtonSymbols(QAbstractSpinBox.NoButtons)

        self.dutycyclespin = QDoubleSpinBox()
        self.dutycyclespin.setReadOnly(True)
        self.dutycyclespin.setSuffix('%')
        self.dutycyclespin.setButtonSymbols(QAbstractSpinBox.NoButtons)

        self.ambientspin = QDoubleSpinBox()
        self.ambientspin.setReadOnly(True)
        self.ambientspin.setSuffix(degreesC)
        self.ambientspin.setButtonSymbols(QAbstractSpinBox.NoButtons)

        self.involtagespin = QDoubleSpinBox()
        self.involtagespin.setReadOnly(True)
        self.involtagespin.setSuffix('V')
        self.involtagespin.setButtonSymbols(QAbstractSpinBox.NoButtons)

        # set up external connections
        connmgr.addSignal(self.timer.timeout, 'RefreshMonitor')
        connmgr.addSignal(self.lampstateupdate, 'LampEnableChanged')
        connmgr.addSignal(self.newlogmessage, 'WriteToLog')
        connmgr.addSignal(self.printbootstatus, 'PrintBootStatus')
        connmgr.addSlot(self.doDeviceStartAction, 'DeviceStarted')
        connmgr.addSlot(self.stopMonitor, 'SerialClosed')
        connmgr.addSlot(self.setMonitorData, 'MonitorRefreshed')

        # set up our control layout
        # For some reason I have to put the Form layout inside a VBox layout to get it to center on
        # the page.
        self.vbox = QVBoxLayout(self)
        self.formlo = QFormLayout()

        self.vbox.setAlignment(Qt.AlignCenter)
        self.vbox.addLayout(self.formlo)
        self.formlo.setAlignment(Qt.AlignCenter)
        self.formlo.addRow('Sensor Temperature', self.sensortempspin)
        self.formlo.addRow('Current Fan Speed', self.dutycyclespin)
        self.formlo.addRow('Ambient Temperature', self.ambientspin)
        self.formlo.addRow('12V Input Supply', self.involtagespin)

    @QtCore.Slot(tuple)
    def setMonitorData(self, monitordata):
        self.involtagespin.setValue(_get12VFromADC(monitordata[0][0]))
        self.ambientspin.setValue(_getAmbientFromADC(monitordata[0][1]))
        self.sensortempspin.setValue(_getThermistorTempFromADC(monitordata[0][2]))
        self.dutycyclespin.setValue(monitordata[1])
        
        self.lampstateupdate.emit(monitordata[2] != 0)

        _printErrorFromCode(monitordata[3])

    @QtCore.Slot(bool)
    def doDeviceStartAction(self, isApp):
        if isApp:
            self.timer.start()
        else:
            self.timer.stop()

        self.printbootstatus.emit()

    @QtCore.Slot()
    def stopMonitor(self):
        self.timer.stop()

    def _print(self, text):
        self.newlogmessage.emit(text)

    def _get12VFromADC(self, adcreading):
        Vadc = (5.0 * adcreading) / 1024.0
        return 4.03 * Vadc

    def _getAmbientFromADC(self, adcreading):
        Vadc = (5.0 * adcreading) / 1024.0
        return 100 * (Vadc - 0.5)

    def _getThermistorTempFromADC(self, adcreading):
        # This is a 13-bit signed ADC that that the firmware extends to 16 bits, but it needs to be
        # extended again since Python integers are infinite-width.
        if adcreading & 0x8000:
            adcreading = ((adcreading * -1) & 0xFFFF) * -1

        # clamp input to valid domain (-4096 indicates missing thermistor; firmware handles that)
        if adcreading > 4095:
            adcreading = 4095
        elif adcreading < -4095:
            adcreading = -4095

        Vadc = (2.5 * adcreading) / 4096 + 2.5
        R = (5.0 / Vadc) - 1.0
        lnR = math.log(R)

        T = 1.0 / (THM_A + THM_B*lnR + THM_C*(lnR**2) + THM_D*(lnR**3))
        T -= 273.15

        return T

    def _printErrorFromCode(self, errcode):
        if errcode == errorNone:
            pass
        elif errcode == errorOvertemp:
            self._print('Temperature over limit; lamp disabled.')
        elif errcode == errorThmShort:
            self._print('Temperature sensor shorted; lamp disabled.')
        elif errcode == errorThmOpen:
            self._print('Temperature sensor missing; lamp disabled.')
        else:
            self._printf('Received unrecognized error code ' + hex(errcode) + '.')

