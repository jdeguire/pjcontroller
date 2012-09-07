#! /usr/bin/env python

"""
main.py

The entry point for the controller app.
"""

import sys

from PySide.QtCore import QThread
from PySide.QtGui import QApplication

import serialcomm
import connmanager
import mainwindow


def main(argv = None):
    if argv is None:
        argv = sys.argv

    connmgr = connmanager.ConnectionManager()
    comm = serialcomm.SerialComm(connmgr)
    app = QApplication(argv)
    appwindow = mainwindow.MainWindow(connmgr)
    commthread = QThread()

    comm.enumerateSerialPorts()
    comm.moveToThread(commthread)
    commthread.start()
    appwindow.show()

    result = app.exec_()
    commthread.quit()
    commthread.wait()

    return result


if __name__ == '__main__':
    sys.exit(main())
