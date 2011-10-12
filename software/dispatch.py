#! /usr/bin/env python

"""
dispatch.py

Defines a class for a message dispatcher, which will take care of queue management for threads.
"""

import Queue


class Dispatcher:
    """Handle queue management for a thread.

    This will act as a message hub for a thread.  Data is sent out through the dispatcher and
    incoming data can be registered so that the dispatcher will take care of handling the command
    properly.
    """

    def __init__(self, outgoingQ, incomingQ):
        """Create a new Dispatcher.

        Parameters are the queues through which commands will be sent through and results received
        from.
        """ 
        self.outgoingQ = outgoingQ
        self.incomingQ = incomingQ
        self.handlers = {}

    def registerHandler(self, cmd, handler):
        """Register a new command handler with the Dispatcher.

        This will allow the dispatcher to handle incoming commands for the thread by binding the
        command to the given handler.  Multiple handlers can exist for a single command,
        in which case all of them will be called when the command comes in.
        """
        # Don't recognize that command?  Create an entry for it.
        if not self.handlers.has_key(cmd):
            self.handlers[cmd] = []

        # Now add the handler to the list for that command
        self.handlers[cmd].append(handler)

    def dispatch(self):
        """Dispatch the next incoming command to its handlers.  

        Return True if a command was available or False if it was not.
        """
        if not self.incomingQ.empty():
            response = self.incomingQ.get()
            for fn in self.handlers[response[0]]:
                fn(response[1])
            return True

        return False

    def send(self, cmd, data):
        """Send a new command out with the given data.
        """
        self.outgoingQ.put((cmd, data))

