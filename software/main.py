#! /usr/bin/env python

"""
main.py

The entry point for the controller app.
"""

import sys
import pages
import commthread
from PySide import QtCore
from PySide.QtCore import *
from PySide.QtGui import *

class MainWindow(QDialog):
    serialopenclicked = QtCore.Signal(str)

    def __init__(self):
        QDialog.__init__(self)
        self.setWindowTitle("Test App")

        self.tabwidget = QTabWidget()
        self.updatepage = pages.UpdatePage()

        self.serialcombo = QComboBox()
        self.serialopenbutton = QPushButton('Open')
        self.serialrefreshbutton = QPushButton('Refresh')

        self.serialcombo.setInsertPolicy(QComboBox.InsertPolicy.InsertAtTop)
        self.serialcombo.setEditable(True)

        self.tabwidget.addTab(self.updatepage, "Update")

        # alias existing signals so they're easier to access externally
        self.serialrefreshclicked = self.serialrefreshbutton.clicked

        # set up our control layout
        self.vbox = QVBoxLayout(self)
        self.serialhbox = QHBoxLayout()
        self.vbox.addLayout(self.serialhbox)
        self.serialhbox.addWidget(self.serialcombo)
        self.serialhbox.addWidget(self.serialopenbutton)
        self.serialhbox.addWidget(self.serialrefreshbutton)
        self.vbox.addWidget(self.tabwidget)

        # connect signals to internal slots
        self.serialopenbutton.clicked.connect(self.openSerial)

    @QtCore.Slot(str)
    def printText(self, text):
        print text

    @QtCore.Slot()
    def openSerial(self):
        self.serialopenclicked.emit(self.serialcombo.currentText())

    @QtCore.Slot(list)
    def setSerialPortChoices(self, portlist):
        self.serialcombo.clear()
        self.serialcombo.addItems(portlist)


def main(argv = None):
    if argv is None:
        argv = sys.argv

    comm = commthread.CommThread()
    app = QApplication(argv)
    mainwindow = MainWindow()

    # connect signals and slots between UI and Comm Thread
    mainwindow.updatepage.updatestartclicked.connect(comm.doFirmwareUpdate)
    mainwindow.serialopenclicked.connect(comm.openSerialPort)
    mainwindow.serialrefreshclicked.connect(comm.enumerateSerialPorts)
    comm.updateprogressed.connect(mainwindow.updatepage.setUpdateProgress)
    comm.updatecompleted.connect(mainwindow.updatepage.endUpdate)
    comm.serialenumerated.connect(mainwindow.setSerialPortChoices)
    comm.newtextmessage.connect(mainwindow.printText)

    comm.start()
    comm.enumerateSerialPorts()
    mainwindow.show()
    
    result = app.exec_()
    comm.quit()
    comm.wait()

    return result


if __name__ == '__main__':
    sys.exit(main())
