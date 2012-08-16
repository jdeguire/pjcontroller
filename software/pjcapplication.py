#! /usr/bin/env python


""" pjcapplication.py


Contains a class that implements commands specific to the PJC Application.  As a child of the
PJCInterface class, this is used to communicate to the ATMega device over the serial port.
"""

import pjcinterface

class PJCApplication(pjcinterface.PJCInterface):
    """An interface to the main PJC application on the ATMega device.

    This implements application-specific commands, such turning on the lamp, reading the ADC, or
    getting the states of digital IO.
    """

    def readADCs(self):
        """Read the ADCs on the board, returning them as a list of integers.
        """
        return self.execCommand('a', pjcinterface.PJCInterface.RespHexList)

    def enableLamp(self, en):
        """Enable or disable the lamp and its control loop, returning True if the lamp was enabled
        or False if it was disabled.
        """
        return self.execCommand('le ' + str(int(en)), pjcinterface.PJCInterface.RespDecimal) != 0

    def isLampEnabled(self):
        """Return True if the lamp is on and being controlled or False otherwise.
        """
        return self.execCommand('le', pjcinterface.PJCInterface.RespDecimal) != 0

    def setOvertempLimit(self, limit):
        """Set the temperature at which the firmware will shut down the lamp and return the new
        limit.
        """
        return self.execCommand('ot ' + str(limit), pjcinterface.PJCInterface.RespFloat)

    def getOvertempLimit(self):
        """Get the temperature at which the firmware will shut the lamp down in order to protect
        projector components.
        """
        return self.execCommand('ot', pjcinterface.PJCInterface.RespFloat)

    def setMinDutyCycle(self, mindc):
        """Set the minimum duty cycle in percent for the PWM controlling the fan speed and return
        the new duty cycle.
        """
        return self.execCommand('dcl ' + str(mindc), pjcinterface.PJCInterface.RespFloat)

    def getMinDutyCycle(self, mindc):
        """Get the minimum duty cycle in percent for the PWM controlling the fan speed.
        """
        return self.execCommand('dcl', pjcinterface.PJCInterface.RespFloat)
