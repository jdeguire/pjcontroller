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

    def __init__(self):
        PageBase.__init__(self)

        # widgets in the dialog box
        self.fileline = QLineEdit('Select hex file...')
        self.browsebutton = QPushButton('Browse...')
        self.startbutton = QPushButton('Start')
        self.progress = QProgressBar()
        self.progress.setRange(0, 100)

        # so our file dialog remembers where we last were (default to home directory)
        self.lasthexdir = os.path.expanduser('~')

        # put the widgets into a vertical layout
        self.vbox = QVBoxLayout(self)
        self.vbox.addWidget(self.fileline)
        self.vbox.addWidget(self.browsebutton)
        self.vbox.addWidget(self.progress)
        self.vbox.addWidget(self.startbutton)

        # connect signals to internal slots
        self.browsebutton.clicked.connect(self.browseForHexFile)
        self.startbutton.clicked.connect(self.startNewUpdate)

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

    @QtCore.Slot(int)
    def setUpdateProgress(self, prog):
        self.progress.setValue(prog)

    @QtCore.Slot(int)
    def endUpdate(self, status):
        self.progress.reset()
        # may want to handle the status code at some point...
