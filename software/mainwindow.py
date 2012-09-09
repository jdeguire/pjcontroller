#! /usr/bin/env python

"""
mainwindow.py

Contains the MainWindow class.
"""

from PySide import QtCore
from PySide.QtCore import *
from PySide.QtGui import *

import connmanager
import monitorpage
import settingspage
import updatepage


class MainWindow(QDialog):
    serialopenclicked = QtCore.Signal(str)

    def __init__(self, connmgr):
        QDialog.__init__(self)
        self.setWindowTitle('Test App')

        self.serialcombo = QComboBox()
        self.serialcombo.setInsertPolicy(QComboBox.InsertPolicy.InsertAtTop)
        self.serialcombo.setEditable(True)
        self.serialcombo.setMinimumWidth(180)

        self.serialopenbutton = QPushButton('Open')
        self.serialopenbutton.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Fixed)

        self.serialrefreshbutton = QPushButton('Refresh')
        self.serialrefreshbutton.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Fixed)

        self.monitorpage = monitorpage.MonitorPage(connmgr)
        self.settingspage = settingspage.SettingsPage(connmgr)
        self.updatepage = updatepage.UpdatePage(connmgr)

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