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
# with Projector Controller.  If not, see <http://www.gnu.org/licenses/>

"""
File:   updatepage.py
Author: Jesse DeGuire

Contains the UpdatePage class.
"""

import os
import hashlib

from PySide import QtCore
from PySide.QtCore import *
from PySide.QtGui import *

from connmanager import ConnectionManager


class UpdatePage(QDialog):
    """The page used for performing firmware updates to the device.
    """
    # new signals have to be declared out here, something the docs aren't very explicit about
    updatestartclicked = QtCore.Signal(str)

    def __init__(self, connmgr):
        QDialog.__init__(self)

        # widgets in the dialog box
        self.fileline = QLineEdit()
        self.fileline.setPlaceholderText('Select hex file...')

        self.browsebutton = QPushButton('...')
        
        # Set the appropriate size manually since the "standard" size is too big.
        # It seems that buttons get a 10 pixel pad on each side.
        browsefw = self.browsebutton.fontMetrics().width(self.browsebutton.text())
        if browsefw > 15:
            self.browsebutton.setFixedWidth(browsefw + 20)
        else:
            self.browsebutton.setFixedWidth(35)

        self.hashlabel = QLabel("MD5 Sum")
        self.hashline = QLineEdit()
        self.hashline.setPlaceholderText('No file selected')
        self.hashline.setReadOnly(True)

        self.startbutton = QPushButton('Start')
        self.startbutton.setSizePolicy(QSizePolicy.Fixed, QSizePolicy.Fixed)

        self.progress = QProgressBar()
        self.progress.setRange(0, 100)
        self.progress.setFixedWidth(100)

        # so our file dialog remembers where we last were (default to home directory)
        self.lasthexdir = os.path.expanduser('~')

        # set up external connections
        connmgr.addSignal(self.updatestartclicked, 'StartUpdate')
        connmgr.addSlot(self.setUpdateProgress, 'UpdateProgressed')
        connmgr.addSlot(self.endUpdate, 'UpdateCompleted')

        # connect signals to internal slots
        self.browsebutton.clicked.connect(self.browseForHexFile)
        self.startbutton.clicked.connect(self.startNewUpdate)

        # set up our control layout
        self.vbox = QVBoxLayout(self)
        self.filehbox = QHBoxLayout()
        self.starthbox = QHBoxLayout()

        self.vbox.setAlignment(Qt.AlignCenter)
        self.vbox.addLayout(self.filehbox)
        self.filehbox.addWidget(self.fileline)
        self.filehbox.addWidget(self.browsebutton)
        self.vbox.addLayout(self.starthbox)
        self.starthbox.setAlignment(Qt.AlignLeft)
        self.starthbox.addWidget(self.startbutton)
        self.starthbox.addWidget(self.progress)
        self.vbox.addSpacing(10)
        self.vbox.addWidget(self.hashlabel)
        self.vbox.addWidget(self.hashline)

    @QtCore.Slot()
    def browseForHexFile(self):
        hexpath = QFileDialog.getOpenFileName(self, 'Select hex file', self.lasthexdir,
                                              'Intel hex files (*.hex);;All Files (*)')

        if hexpath[0] != '':
            self.fileline.setText(hexpath[0])
            self.lasthexdir = os.path.dirname(hexpath[0])
            
            h = hashlib.md5()

            with open(hexpath[0], 'r') as hexfile:
                for line in hexfile:
                    h.update(line)

            self.hashline.setText(h.hexdigest())

    @QtCore.Slot()
    def startNewUpdate(self):
        self.progress.reset()
        self.updatestartclicked.emit(self.fileline.text())

    @QtCore.Slot(int)
    def setUpdateProgress(self, prog):
        self.progress.setValue(prog)

    @QtCore.Slot(bool)
    def endUpdate(self, result):
        self.progress.reset()
