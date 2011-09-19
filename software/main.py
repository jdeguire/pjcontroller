#! /usr/bin/env python

"""
main.py

Will eventually become the entry point for the controller app.
Now, it's just a way for me to experiment with PySide.
"""

import sys
import serial
import os
import glob
import pjcbootloader
from PySide.QtCore import *
from PySide.QtGui import *


class UpdatePage:
    
    def __init__(self, parent = None):
        # create our main dialog box and set its title
        self.dialog = QDialog(parent)
        self.dialog.setWindowTitle('Update Box')

        # widgets in the dialog box
        self.serialCombo = QComboBox()
        self.fileline = QLineEdit('Select hex file...')
        self.browsebutton = QPushButton('Browse...')
        self.progress = QProgressBar()
        self.startbutton = QPushButton('Start')

        self.progress.setMinimum(0)
        self.serialCombo.setInsertPolicy(QComboBox.InsertPolicy.InsertAtTop)
        self.populateSerialComboBox()

        # so our file dialog remembers where we last were (default to home directory)
        self.lasthexdir = os.path.expanduser('~')

        # put the widgets into a vertical layout
        layout = QVBoxLayout()
        layout.addWidget(self.serialCombo)
        layout.addWidget(self.fileline)
        layout.addWidget(self.browsebutton)
        layout.addWidget(self.progress)
        layout.addWidget(self.startbutton)
        self.dialog.setLayout(layout)   # can also pass 'dialog' to the layout's constructor

        # connect signals from buttons to slots
        self.browsebutton.clicked.connect(self.browseForHexFile)
        self.startbutton.clicked.connect(self.doFirmwareUpdate)

    def populateSerialComboBox(self):
        # Still need to make something to work in Windows...
        for f in glob.glob('/dev/tty*'):
            try:
                s = serial.Serial(f)
                self.serialCombo.addItem(s.name)
                s.close()
            except serial.SerialException:
                pass

    def browseForHexFile(self):
        hexfile = QFileDialog.getOpenFileName(self.dialog, 'Select hex file', self.lasthexdir,
                                              'Intel hex files (*.hex);;All Files (*)')

        if hexfile[0] != '':
            self.fileline.setText(hexfile[0])
            self.lasthexdir = os.path.dirname(hexfile[0])

    def doFirmwareUpdate(self):
        # later we'll need to create a selection box for the serial device
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


    def show(self):
        self.dialog.show()


if __name__ == '__main__':
    app = QApplication(sys.argv)
    page = UpdatePage()
    page.show()
    
    sys.exit(app.exec_())
        
