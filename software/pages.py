#! /usr/bin/env python

"""
pages.py

Defines classes for each page of the application.  The main app will have a tab control and each tab
will contain one of these pages.
"""

import serial
import os
import glob
import pjcbootloader
from PySide.QtCore import *
from PySide.QtGui import *


class PageBase:
    """Base class for the pages that can appear in the main tab window.
    """
    
    def __init__(self, dispatcher):
        self.container = QDialog()
        self.dispatcher = dispatcher

    def widget(self):
        return self.container;


class UpdatePage(PageBase):
    """The page used for performing firmware updates to the device.
    """

    def __init__(self, dispatcher):
        PageBase.__init__(self, dispatcher)

        # widgets in the dialog box
        self.serialCombo = QComboBox()
        self.fileline = QLineEdit('Select hex file...')
        self.browsebutton = QPushButton('Browse...')
        self.progress = QProgressBar()
        self.startbutton = QPushButton('Start')

        self.progress.setMinimum(0)
        self.serialCombo.setInsertPolicy(QComboBox.InsertPolicy.InsertAtTop)
        self.serialCombo.setEditable(True)
        self.populateSerialComboBox()

        # so our file dialog remembers where we last were (default to home directory)
        self.lasthexdir = os.path.expanduser('~')

        # put the widgets into a vertical layout
        self.layout = QVBoxLayout(self.container)
        self.layout.addWidget(self.serialCombo)
        self.layout.addWidget(self.fileline)
        self.layout.addWidget(self.browsebutton)
        self.layout.addWidget(self.progress)
        self.layout.addWidget(self.startbutton)

        # connect signals from buttons to slots
        self.browsebutton.clicked.connect(self.browseForHexFile)
        self.startbutton.clicked.connect(self.doFirmwareUpdate)

    def populateSerialComboBox(self):
        for i in range(256):
           try:
                s = serial.Serial(i)
                self.serialCombo.addItem(s.name)
                s.close()
           except serial.SerialException:
                pass

    def browseForHexFile(self):
        hexfile = QFileDialog.getOpenFileName(self.container, 'Select hex file', self.lasthexdir,
                                              'Intel hex files (*.hex);;All Files (*)')

        if hexfile[0] != '':
            self.fileline.setText(hexfile[0])
            self.lasthexdir = os.path.dirname(hexfile[0])

    def doFirmwareUpdate(self):
        pjc = pjcbootloader.PJCBootloader(self.serialCombo.currentText())
        self.progress.reset()
        
        # not final, doesn't handle exceptions and stuff
        if pjc.getBootloaderVersion() >= 0:
            if pjc.parseFile(self.fileline.text()):
                pjc.getMaxPages()
                pjc.getPageSize()
                
                numpages = pjc.getFileNumPages()
                self.progress.setMaximum(numpages)

                print 'File CRC: ' + hex(pjc.calculateFileCRC())
                print 'File pages: ' + str(numpages) + '\n'
                
                print 'Erasing old app...'
                pjc.eraseApp()

                print 'Loading new app:',

                for i in range(numpages):
                    pageresult = pjc.programPage(i)

                    if 0 == pageresult:
                        self.progress.setValue(i + 1)
                    else:
                        print '\nFailed to program page ' + str(i)
                        break;

                if 0 == pageresult:
                    pjc.writeCRC()
                    print 'Update complete!'
                else:
                    print 'Update returned error code ' + str(pageresult)
            else:
                print 'File parse failed'
        else:
            print 'Could not communicate with bootloader'
