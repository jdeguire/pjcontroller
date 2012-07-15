#! /usr/bin/env python

"""
connmanager.py

Contains the ConnectionManager class, which makes it easier to pair signals with slots.
"""

import PySide.QtCore

class ConnectionManager:
    """Keep track of signals and slots and help pair them together.

    This keeps a list of signals, slots, and names associated with them.  A caller can store a
    signal or slot in an instance of this class and give it a name.  This will then look for
    the slot or signal of the same name and connect them together.  The class allows objects to
    advertise their signals or slots without actually having to actually know where to connect them.
    """

    def __init__(self):
        self.connections = {}

    def addSignal(self, sig, name):
        """Add a new signal to the list.

        Connect the given named signal to any slots matching that name.  Multiple signals can have
        the same name, allowing different actions to tigger the same response.  Returns the number
        of signals with the given name after the new one has been added.
        """
        if self.connections.has_key(name):
            self.connections[name][0].append(sig)
        else:
            self.connections[name] = ([sig], [])

        map(sig.connect, self.connections[name][1])

        return len(self.connections[name][0])

    def addSlot(self, slt, name):
        """Add a new slot to the list.

        Connect any signals with the given name to the given slot.  Multiple slots can be assigned
        to a signal of the same name, allowing for a single signal to fire off multiple slots.
        Returns the number of slots with the given name after the new one has been added.
        """
        if self.connections.has_key(name):
            self.connections[name][1].append(slt)
        else:
            self.connections[name] = ([], [slt])

        map(lambda s: s.connect(slt), self.connections[name][0])

        return len(self.connections[name][1])
