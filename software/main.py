#! /usr/bin/env python

"""
main.py

The entry point for the controller app.
"""

import sys
import pages
import commthread
from PySide.QtCore import *
from PySide.QtGui import *

class MainWindow:
    def __init__(self):
        self.tabwidget = QTabWidget()
        self.updatepage = pages.UpdatePage()

        self.tabwidget.setWindowTitle("Test App")
        self.tabwidget.addTab(self.updatepage, "Update")

    def show(self):
        self.tabwidget.show()

    def printText(self, text):
        print text

def main(argv = None):
    if argv is None:
        argv = sys.argv

    comm = commthread.CommThread()
    app = QApplication(argv)
    mainwindow = MainWindow()

    # connect signals and slots between UI and Comm Thread
    mainwindow.updatepage.updatestartclicked.connect(comm.doFirmwareUpdate)
    mainwindow.updatepage.serialopenclicked.connect(comm.openSerialPort)
    mainwindow.updatepage.serialrefreshclicked.connect(comm.enumerateSerialPorts)
    comm.updateprogressed.connect(mainwindow.updatepage.setUpdateProgress)
    comm.updatecompleted.connect(mainwindow.updatepage.endUpdate)
    comm.serialenumerated.connect(mainwindow.updatepage.setSerialPortChoices)
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
