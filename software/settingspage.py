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
File:   settingspage.py
Author: Jesse DeGuire

Contains the SettingsPage class.
"""

from PySide import QtCore
from PySide.QtCore import *
from PySide.QtGui import *

from connmanager import ConnectionManager


class SettingsPage(QDialog):
    """The page used for changing device settings, such as target temperature or minimum fan speed.
    """
    targettempmodified = QtCore.Signal(float)
    overtempmodified = QtCore.Signal(float)
    fanoffmodified = QtCore.Signal(float)
    lampdelaymodified = QtCore.Signal(float)
    mindutycyclemodified = QtCore.Signal(float)

    def __init__(self, connmgr):
        QDialog.__init__(self)

        degreesC = unichr(176).encode("latin-1") + 'C'

        # widgets in the dialog box
        self.lampcheck = QCheckBox('Lamp Enabled')

        # used to set controller target temperature
        self.targettempspin = QDoubleSpinBox()
        self.targettempspin.setSuffix(degreesC)
        self.targettempspin.setRange(0, 125)

        # used to set over-temp limit
        self.overtempspin = QDoubleSpinBox()
        self.overtempspin.setSuffix(degreesC)
        self.overtempspin.setRange(0, 125)

        # used to set fan off setpoint (relative to ambient)
        self.fanoffspin = QDoubleSpinBox()
        self.fanoffspin.setSuffix(degreesC)
        self.fanoffspin.setRange(0, 125)

        # used to set lamp off delay time
        self.lampdelayspin = QDoubleSpinBox()
        self.lampdelayspin.setSuffix('s')
        self.lampdelayspin.setRange(0, 60)

        # used to set minimum duty cycle
        self.mindutycyclespin = QDoubleSpinBox()
        self.mindutycyclespin.setSuffix('%')
        self.mindutycyclespin.setRange(0, 100)

        self.savebutton = QPushButton('Save')

        self.refreshbutton = QPushButton('Refresh')

        # set up external connections
        connmgr.addSignal(self.lampcheck.stateChanged, 'SetLampEnable')
        connmgr.addSignal(self.targettempmodified, 'SetTargetTemp')
        connmgr.addSignal(self.overtempmodified, 'SetOvertemp')
        connmgr.addSignal(self.fanoffmodified, 'SetFanOffTemp')
        connmgr.addSignal(self.lampdelaymodified, 'SetLampOffDelay')
        connmgr.addSignal(self.mindutycyclemodified, 'SetMinDutyCycle')
        connmgr.addSignal(self.refreshbutton.clicked, 'RefreshSettings')
        connmgr.addSignal(self.savebutton.clicked, 'SaveSettings')
        connmgr.addSlot(self.lampcheck.setCheckState, 'LampEnableChanged')
        connmgr.addSlot(self.targettempspin.setValue, 'TargetTempChanged')
        connmgr.addSlot(self.overtempspin.setValue, 'OvertempChanged')
        connmgr.addSlot(self.fanoffspin.setValue, 'FanOffTempChanged')
        connmgr.addSlot(self.lampdelayspin.setValue, 'LampOffDelayChanged')
        connmgr.addSlot(self.mindutycyclespin.setValue, 'MinDutyCycleChanged')
        connmgr.addSlot(self.doDeviceStartAction, 'DeviceStarted')

        # connect signals to internal slots
        self.targettempspin.editingFinished.connect(self.setTargetTemp)
        self.overtempspin.editingFinished.connect(self.setOvertemp)
        self.fanoffspin.editingFinished.connect(self.setFanOffTemp)
        self.lampdelayspin.editingFinished.connect(self.setLampOffDelay)
        self.mindutycyclespin.editingFinished.connect(self.setMinDutyCycle)

        # set up our control layout
        self.vbox = QVBoxLayout(self)
        self.formlo = QFormLayout()
        self.buttonhbox = QHBoxLayout()

        self.vbox.setAlignment(Qt.AlignCenter)
        self.vbox.addWidget(self.lampcheck)
        self.vbox.setAlignment(self.lampcheck, Qt.AlignHCenter)
        self.vbox.addSpacing(10)
        self.vbox.addLayout(self.formlo)
        self.formlo.setAlignment(Qt.AlignHCenter)
        self.formlo.addRow('Target Temperature', self.targettempspin)
        self.formlo.addRow('Overtemp Limit', self.overtempspin)
        self.formlo.addRow('Fan Shutoff Point', self.fanoffspin)
        self.formlo.addRow('Lamp Off Delay', self.lampdelayspin)
        self.formlo.addRow('Minimum Fan Speed', self.mindutycyclespin)
        self.vbox.addSpacing(10)
        self.vbox.addLayout(self.buttonhbox)
        self.buttonhbox.setAlignment(Qt.AlignCenter)
        self.buttonhbox.addWidget(self.savebutton)
        self.buttonhbox.addWidget(self.refreshbutton)

    @QtCore.Slot(float)
    def setTargetTemp(self):
        self.targettempmodified.emit(self.targettempspin.value())

    @QtCore.Slot(float)
    def setOvertemp(self):
        self.overtempmodified.emit(self.overtempspin.value())

    @QtCore.Slot(float)
    def setFanOffTemp(self):
        self.fanoffmodified.emit(self.fanoffspin.value())

    @QtCore.Slot(float)
    def setLampOffDelay(self):
        self.lampdelaymodified.emit(self.lampdelayspin.value())

    @QtCore.Slot(float)
    def setMinDutyCycle(self):
        self.mindutycyclemodified.emit(self.mindutycyclespin.value())

    @QtCore.Slot(bool)
    def doDeviceStartAction(self, isApp):
        if isApp:
            self.refreshbutton.click()
