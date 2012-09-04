#! /usr/bin/env python

"""
settingspage.py

Contains the SettingsPage class.
"""

import connmanager
from PySide import QtCore
from PySide.QtCore import *
from PySide.QtGui import *


class SettingsPage(QDialog):
    """The page used for changing device settings, such as target temperature or minimum fan speed.
    """
    targettempmodified = QtCore.Signal(float)
    overtempmodified = QtCore.Signal(float)
    fanoffmodified = QtCore.Signal(float)
    mindutycyclemodified = QtCore.Signal(float)

    def __init__(self, connmgr):
        QDialog.__init__(self)

        self.doublevalidator = QDoubleValidator()

        # widgets in the dialog box
        self.lampcheck = QCheckBox('Enable lamp')

        self.targettempline = QLineEdit()
        self.targettempline.setValidator(self.doublevalidator)

        self.overtempline = QLineEdit()
        self.overtempline.setValidator(self.doublevalidator)

        self.fanoffline = QLineEdit()
        self.fanoffline.setValidator(self.doublevalidator)

        self.mindutycycleline = QLineEdit()
        self.mindutycycleline.setValidator(self.doublevalidator)

        self.savebutton = QPushButton('Save')
        self.savebutton.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Fixed)

        self.refreshbutton = QPushButton('Refresh')
        self.refreshbutton.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Fixed)

        # set up external connections
        connmgr.addSignal(self.lampcheck.stateChanged, 'SetLampEnable')
        connmgr.addSignal(self.targettempmodified, 'SetTargetTemp')
        connmgr.addSignal(self.overtempmodified, 'SetOvertemp')
        connmgr.addSignal(self.fanoffmodified, 'SetFanOffTemp')
        connmgr.addSignal(self.mindutycyclemodified, 'SetMinDutyCycle')
        connmgr.addSignal(self.refreshbutton.clicked, 'RefreshSettings')
        connmgr.addSignal(self.savebutton.clicked, 'SaveSettings')
        connmgr.addSlot(self.lampcheck.setCheckState, 'LampEnableChanged')
        connmgr.addSlot(self.updateTargetTempLine, 'TargetTempChanged')
        connmgr.addSlot(self.updateOvertempLine, 'OvertempChanged')
        connmgr.addSlot(self.updateFanOffLine, 'FanOffTempChanged')
        connmgr.addSlot(self.updateMinDutyCycleLine, 'MinDutyCycleChanged')
        connmgr.addSlot(self.doDeviceStartAction, 'DeviceStarted')

        # connect signals to internal slots
        # Note that the editingFinished signal fires only if the validator attached to the control
        # accepts the input
        self.targettempline.editingFinished.connect(self.setTargetTemp)
        self.overtempline.editingFinished.connect(self.setOvertemp)
        self.fanoffline.editingFinished.connect(self.setFanOffTemp)
        self.mindutycycleline.editingFinished.connect(self.setMinDutyCycle)

        # set up our control layout
        self.vbox = QVBoxLayout(self)
        self.checkhbox = QHBoxLayout()
        self.formlo = QFormLayout()
        self.buttonhbox = QHBoxLayout()
        self.vbox.addStretch()
        self.vbox.addLayout(self.checkhbox)
        self.checkhbox.addStretch()
        self.checkhbox.addWidget(self.lampcheck)
        self.checkhbox.addStretch()
        self.vbox.addLayout(self.formlo)
        self.formlo.addRow('Target temperature', self.targettempline)
        self.formlo.addRow('Overtemp Limit', self.overtempline)
        self.formlo.addRow('Fan Shutoff Point', self.fanoffline)
        self.formlo.addRow('Minimum Fan Speed', self.mindutycycleline)
        self.vbox.addLayout(self.buttonhbox)
        self.buttonhbox.addStretch()
        self.buttonhbox.addWidget(self.savebutton)
        self.buttonhbox.addWidget(self.refreshbutton)
        self.buttonhbox.addStretch()
        self.vbox.addStretch()

    @QtCore.Slot(float)
    def setTargetTemp(self):
        if self.targettempline.isModified():
            self.targettempline.setModified(False)
            self.targettempmodified.emit(float(self.targettempline.text()))

    @QtCore.Slot(float)
    def setOvertemp(self):
        if self.overtempline.isModified():
            self.overtempline.setModified(False)
            self.overtempmodified.emit(float(self.overtempline.text()))

    @QtCore.Slot(float)
    def setFanOffTemp(self):
        if self.fanoffline.isModified():
            self.fanoffline.setModified(False)
            self.fanoffmodified.emit(float(self.fanoffline.text()))

    @QtCore.Slot(float)
    def setMinDutyCycle(self):
        if self.mindutycycleline.isModified():
            self.mindutycycleline.setModified(False)
            self.mindutycyclemodified.emit(float(self.mindutycycleline.text()))

    @QtCore.Slot(float)
    def updateTargetTempLine(self, newtemp):
        self.targettempline.setText(str(newtemp))

    @QtCore.Slot(float)
    def updateOvertempLine(self, newtemp):
        self.overtempline.setText(str(newtemp))

    @QtCore.Slot(float)
    def updateFanOffLine(self, newvalue):
        self.fanoffline.setText(str(newvalue))

    @QtCore.Slot(float)
    def updateMinDutyCycleLine(self, newdc):
        self.mindutycycleline.setText(str(newdc))

    @QtCore.Slot(bool)
    def doDeviceStartAction(self, isApp):
        if isApp:
            self.refreshbutton.click()
