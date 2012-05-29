#! /usr/bin/env python

"""
main.py

Will eventually become the entry point for the controller app.
Now, it's just a way for me to experiment with PySide.
"""

import sys
import pages
import dispatch
from PySide.QtCore import *
from PySide.QtGui import *

import time
import threading
import Queue

class CommThread(threading.Thread):
    """Class for the serial communications thread.

    This class communicates with the board over a serial connection in its own thread.  Serial I/O
    is blocking, so a separate thread is needed so that the UI doesn't freeze.  The UI thread will
    put commands into a command queue.  This thread will receive those commands, do them, and reply by
    putting a response in a reply queue.  The UI thread will then need to periodically check the
    reply queue for a response.

    Credit for the architecture goes here:
    http://eli.thegreenplace.net/2011/05/18/code-sample-socket-client-thread-in-python/
    """

    def __init__(self, dispatcher):
        threading.Thread.__init__(self)

        self.alive = threading.Event()
        self.alive.set()

        self.dispatcher = dispatcher
        self.dispatcher.registerHandler(0, self.printIncoming)

    def run(self):
       while self.alive.isSet():
           try:
               if not self.dispatcher.dispatch():
                   time.sleep(0.050)
           except KeyError as e:
               print "Unhandled command " + str(e.message)
               continue

    def join(self, timeout = None):
        self.alive.clear()
        threading.Thread.join(self, timeout)

    def printIncoming(self, stuff):
        print "I got something: " + str(stuff)


class MainWindow:
    def __init__(self, dispatcher):
        self.dispatcher = dispatcher
        self.tabwidget = QTabWidget()
        self.updatepage = pages.UpdatePage(dispatcher)
        
        self.replytimer = QTimer()
        self.counter = 0
        self.replytimer.timeout.connect(self.doTimedThing)
#        self.replytimer.start(1000)

        self.tabwidget.setWindowTitle("Test App")
        self.tabwidget.addTab(self.updatepage.widget(), "Update")
        
    def show(self):
        self.tabwidget.show()

    def doTimedThing(self):
        self.dispatcher.send(0, (self.counter, ))
        self.counter += 1


def main(argv = None):
    if argv is None:
        argv = sys.argv

    ui2serialQ = Queue.Queue()
    serial2uiQ = Queue.Queue()
    
    uidispatcher = dispatch.Dispatcher(ui2serialQ, serial2uiQ)
    serialdispatcher = dispatch.Dispatcher(serial2uiQ, ui2serialQ)

    commthread = CommThread(serialdispatcher)
    commthread.start()

    app = QApplication(argv)

    mainwindow = MainWindow(uidispatcher)
    mainwindow.show()
    
    result = app.exec_()
    commthread.join()

    return result


if __name__ == '__main__':
    sys.exit(main())
