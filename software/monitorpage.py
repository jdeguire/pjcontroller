#! /usr/bin/env python

"""
monitorpage.py

Contains the MonitorPage class.
"""

from PySide import QtCore
from PySide.QtCore import *
from PySide.QtGui import *

from connmanager import ConnectionManager


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
        self.involtagespin.setValue(monitordata[0][0])
        self.ambientspin.setValue(monitordata[0][1])
        self.sensortempspin.setValue(monitordata[0][2])
        self.dutycyclespin.setValue(monitordata[1])
        
        self.lampstateupdate.emit(monitordata[2] != 0)

        if monitordata[3] != 0:
            self._print('Device reported error code ' + str(monitordata[3]) + '.')

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
