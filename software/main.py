#! /usr/bin/env python
#
# Copyright 2011-2013 Jesse DeGuire
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
# with Projector Controller.  If not, see <http://www.gnu.org/licenses/>.

"""
File:   main.py
Author: Jesse DeGuire

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

    version = '20130216'      # modification date in yyyymmdd format

    connmgr = ConnectionManager()
    comm = SerialComm(connmgr)
    app = QApplication(argv)
    appwindow = MainWindow(connmgr, 'Projector Control Panel')
    commthread = QThread()

    comm.enumerateSerialPorts()
    comm.moveToThread(commthread)
    commthread.start()

    appwindow.show()
    appwindow.writeToLog('Software version ' + version + '.')

    result = app.exec_()
    commthread.quit()
    commthread.wait()

    return result


if __name__ == '__main__':
    sys.exit(main())
