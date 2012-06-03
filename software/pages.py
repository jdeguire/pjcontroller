#! /usr/bin/env python

"""
pages.py

Defines classes for each page of the application.  The main app will have a tab control and each tab
will contain one of these pages.
"""

import os
from PySide import QtCore
from PySide.QtCore import *
from PySide.QtGui import *


class PageBase(QDialog):
    """Base class for the pages that can appear in the main tab window.
    """
    
    def __init__(self):
        QDialog.__init__(self)


class UpdatePage(PageBase):
    """The page used for performing firmware updates to the device.
    """

    # new signals have to be declared out here, something the docs aren't very explicit about
    updatestartclicked = QtCore.Signal(str)
    serialopenclicked = QtCore.Signal(str)

    def __init__(self):
        PageBase.__init__(self)

        # widgets in the dialog box
        self.serialcombo = QComboBox()
        self.serialopenbutton = QPushButton('Open')
        self.serialrefreshbutton = QPushButton('Refresh')
        self.fileline = QLineEdit('Select hex file...')
        self.browsebutton = QPushButton('Browse...')
        self.progress = QProgressBar()
        self.startbutton = QPushButton('Start')

        self.progress.setRange(0, 100)
        self.serialcombo.setInsertPolicy(QComboBox.InsertPolicy.InsertAtTop)
        self.serialcombo.setEditable(True)

        # alias existing signals so they're easier to access externally
        self.serialrefreshclicked = self.serialrefreshbutton.clicked

        # so our file dialog remembers where we last were (default to home directory)
        self.lasthexdir = os.path.expanduser('~')

        # put the widgets into a vertical layout
        self.layout = QVBoxLayout(self)
        self.layout.addWidget(self.serialcombo)
        self.serlayout = QHBoxLayout()
        self.layout.addLayout(self.serlayout)
        self.serlayout.addWidget(self.serialopenbutton)
        self.serlayout.addWidget(self.serialrefreshbutton)
        self.layout.addWidget(self.fileline)
        self.layout.addWidget(self.browsebutton)
        self.layout.addWidget(self.progress)
        self.layout.addWidget(self.startbutton)

        # connect signals to internal slots
        self.browsebutton.clicked.connect(self.browseForHexFile)
        self.startbutton.clicked.connect(self.startNewUpdate)
        self.serialopenbutton.clicked.connect(self.openSerial)

    @QtCore.Slot()
    def browseForHexFile(self):
        hexfile = QFileDialog.getOpenFileName(self, 'Select hex file', self.lasthexdir,
                                              'Intel hex files (*.hex);;All Files (*)')

        if hexfile[0] != '':
            self.fileline.setText(hexfile[0])
            self.lasthexdir = os.path.dirname(hexfile[0])

    @QtCore.Slot()
    def startNewUpdate(self):
        self.progress.reset()
        self.updatestartclicked.emit(self.fileline.text())

    @QtCore.Slot()
    def openSerial(self):
        self.serialopenclicked.emit(self.serialcombo.currentText())

    @QtCore.Slot(list)
    def setSerialPortChoices(self, portlist):
        self.serialcombo.clear()
        self.serialcombo.addItems(portlist)

    @QtCore.Slot(int)
    def setUpdateProgress(self, prog):
        self.progress.setValue(prog)

    @QtCore.Slot(int)
    def endUpdate(self, status):
        self.progress.reset()
        # may want to handle the status code at some point...
