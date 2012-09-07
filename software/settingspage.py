#! /usr/bin/env python

"""
settingspage.py

Contains the SettingsPage class.
"""

from PySide import QtCore
from PySide.QtCore import *
from PySide.QtGui import *

import connmanager


class SettingsPage(QDialog):
    """The page used for changing device settings, such as target temperature or minimum fan speed.
    """
    targettempmodified = QtCore.Signal(float)
    overtempmodified = QtCore.Signal(float)
    fanoffmodified = QtCore.Signal(float)
    mindutycyclemodified = QtCore.Signal(float)

    def __init__(self, connmgr):
        QDialog.__init__(self)

        degreesC = unichr(176).encode("latin-1") + 'C'

        # widgets in the dialog box
        self.lampcheck = QCheckBox('Enable lamp')

        self.targettempspin = QDoubleSpinBox()
        self.targettempspin.setSuffix(degreesC)
        self.targettempspin.setRange(0, 125)

        self.overtempspin = QDoubleSpinBox()
        self.overtempspin.setSuffix(degreesC)
        self.overtempspin.setRange(0, 125)

        self.fanoffspin = QDoubleSpinBox()
        self.fanoffspin.setSuffix(degreesC)
        self.fanoffspin.setRange(0, 125)

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
        connmgr.addSignal(self.mindutycyclemodified, 'SetMinDutyCycle')
        connmgr.addSignal(self.refreshbutton.clicked, 'RefreshSettings')
        connmgr.addSignal(self.savebutton.clicked, 'SaveSettings')
        connmgr.addSlot(self.lampcheck.setCheckState, 'LampEnableChanged')
        connmgr.addSlot(self.targettempspin.setValue, 'TargetTempChanged')
        connmgr.addSlot(self.overtempspin.setValue, 'OvertempChanged')
        connmgr.addSlot(self.fanoffspin.setValue, 'FanOffTempChanged')
        connmgr.addSlot(self.mindutycyclespin.setValue, 'MinDutyCycleChanged')
        connmgr.addSlot(self.doDeviceStartAction, 'DeviceStarted')

        # connect signals to internal slots
        self.targettempspin.editingFinished.connect(self.setTargetTemp)
        self.overtempspin.editingFinished.connect(self.setOvertemp)
        self.fanoffspin.editingFinished.connect(self.setFanOffTemp)
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
    def setMinDutyCycle(self):
        self.mindutycyclemodified.emit(self.mindutycyclespin.value())

    @QtCore.Slot(bool)
    def doDeviceStartAction(self, isApp):
        if isApp:
            self.refreshbutton.click()
