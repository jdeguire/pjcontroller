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
File:   mainwindow.py
Author: Jesse DeGuire

Contains the MainWindow class, which creates the main dialog frame that contains the app.
"""

from PySide import QtCore
from PySide.QtCore import *
from PySide.QtGui import *

from connmanager import ConnectionManager
from monitorpage import MonitorPage
from settingspage import SettingsPage
from updatepage import UpdatePage


class MainWindow(QDialog):
    serialopenclicked = QtCore.Signal(str)

    def __init__(self, connmgr, title):
        QDialog.__init__(self)
        self.setWindowTitle(title)

        self.serialcombo = QComboBox()
        self.serialcombo.setInsertPolicy(QComboBox.InsertPolicy.InsertAtTop)
        self.serialcombo.setEditable(True)
        self.serialcombo.setMinimumWidth(180)

        self.serialopenbutton = QPushButton('Open')
        self.serialopenbutton.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Fixed)

        self.serialrefreshbutton = QPushButton('Refresh')
        self.serialrefreshbutton.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Fixed)

        self.monitorpage = MonitorPage(connmgr)
        self.settingspage = SettingsPage(connmgr)
        self.updatepage = UpdatePage(connmgr)

        self.tabwidget = QTabWidget()
        self.tabwidget.addTab(self.monitorpage, 'Monitor')
        self.tabwidget.addTab(self.settingspage, 'Settings')
        self.tabwidget.addTab(self.updatepage, 'Update')

        self.loglabel = QLabel('Log')
        self.logbox = QTextEdit()
        self.logbox.LineWrapMode = QTextEdit.WidgetWidth
        self.logbox.setReadOnly(True)
        self.logbox.setTabChangesFocus(True)
        self.logbox.setVerticalScrollBarPolicy(Qt.ScrollBarAlwaysOn)
        self.logbox.document().setMaximumBlockCount(1024)

        self.logclearbutton = QPushButton('Clear')

        # set up external connections
        connmgr.addSignal(self.serialrefreshbutton.clicked, 'EnumerateSerial')
        connmgr.addSignal(self.serialopenclicked, 'OpenSerial')
        connmgr.addSlot(self.setSerialPortChoices, 'SerialEnumerated')
        connmgr.addSlot(self.logbox.append, 'WriteToLog')

        # connect signals to internal slots
        self.serialopenbutton.clicked.connect(self.openSerial)
        self.logclearbutton.clicked.connect(self.logbox.clear)

        # set up our control layout
        self.vbox = QVBoxLayout(self)
        self.serialhbox = QHBoxLayout()

        self.vbox.addLayout(self.serialhbox)
        self.serialhbox.addWidget(self.serialcombo)
        self.serialhbox.addWidget(self.serialopenbutton)
        self.serialhbox.addWidget(self.serialrefreshbutton)
        self.vbox.addSpacing(10)
        self.vbox.addWidget(self.tabwidget)
        self.vbox.addSpacing(10)
        self.vbox.addWidget(self.loglabel)
        self.vbox.addWidget(self.logbox)
        self.vbox.addWidget(self.logclearbutton)
        self.vbox.setAlignment(self.logclearbutton, Qt.AlignLeft)

    @QtCore.Slot()
    def openSerial(self):
        self.serialopenclicked.emit(self.serialcombo.currentText())

    @QtCore.Slot(list)
    def setSerialPortChoices(self, portlist):
        self.serialcombo.clear()
        self.serialcombo.addItems(portlist)

    def writeToLog(self, text):
        self.logbox.append(text)
