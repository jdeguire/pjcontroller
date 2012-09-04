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

    def setTargetTemperature(self, target):
        """Set the temperature the firmware will attempt to maintain at the thermistor.  The fan
        speed is adjusted to keep this temperature.
        """
        return self.execCommand('tt ' + str(target), pjcinterface.PJCInterface.RespFloat)

    def getTargetTemperature(self):
        """Get the temperature the firmware will attempt to maintain at the thermistor.  The fan
        speed is adjusted to keep this temperature.
        """
        return self.execCommand('tt', pjcinterface.PJCInterface.RespFloat)

    def setFanOffPoint(self, fanoff):
        """Set the point, relative to the ambient temperature, at which the firmware will turn the
        fan off.  This only takes effect if the lamp is off; the fan always runs while the lamp is
        on.
        """
        return self.execCommand('ft ' + str(fanoff), pjcinterface.PJCInterface.RespFloat)

    def getFanOffPoint(self):
        """Get the point, relative to the ambient temperature, at which the firmware will turn the
        fan off.  This only takes effect if the lamp is off; the fan always runs while the lamp is
        on.
        """
        return self.execCommand('ft', pjcinterface.PJCInterface.RespFloat)

    def setMinDutyCycle(self, mindc):
        """Set the minimum duty cycle in percent for the PWM controlling the fan speed and return
        the new duty cycle.
        """
        return self.execCommand('dcl ' + str(mindc), pjcinterface.PJCInterface.RespFloat)

    def getMinDutyCycle(self):
        """Get the minimum duty cycle in percent for the PWM controlling the fan speed.
        """
        return self.execCommand('dcl', pjcinterface.PJCInterface.RespFloat)

    def saveSettingsToEEPROM(self):
        """Save application settings to the onboard EEPROM and return True if successful or False
        otherwise.
        """
        return self.execCommand('sv', pjcinterface.PJCInterface.RespDecimal) != 0
