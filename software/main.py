#! /usr/bin/env python

"""
main.py

The entry point for the controller app.
"""

import sys

from PySide.QtCore import QThread
from PySide.QtGui import QApplication

from serialcomm import SerialComm
from connmanager import ConnectionManager
from mainwindow import MainWindow


def main(argv = None):
    if argv is None:
        argv = sys.argv

    connmgr = ConnectionManager()
    comm = SerialComm(connmgr)
    app = QApplication(argv)
    appwindow = MainWindow(connmgr)
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
