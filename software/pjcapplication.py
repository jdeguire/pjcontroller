#! /usr/bin/env python
#
# Copyright © 2011-2013 Jesse DeGuire
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
File:   pjcapplication.py
Author: Jesse DeGuire

Contains a class that implements commands specific to the PJC Application.  As a child of the
PJCInterface class, this is used to communicate to the ATMega device over the serial port.
"""

from pjcinterface import PJCInterface

class PJCApplication(PJCInterface):
    """An interface to the main PJC application on the ATMega device.

    This implements application-specific commands, such turning on the lamp, reading the ADC, or
    getting the states of digital IO.
    """

    def readADCs(self):
        """Read the ADCs on the board, returning them as a list of integers.
        """
        return self.execCommand('a', PJCInterface.RespHexList)

    def enableLamp(self, en):
        """Enable or disable the lamp and its control loop, returning True if the lamp was enabled
        or False if it was disabled.
        """
        return self.execCommand('le ' + str(int(en)), PJCInterface.RespDecimal) != 0

    def isLampEnabled(self):
        """Return True if the lamp is on and being controlled or False otherwise.
        """
        return self.execCommand('le', PJCInterface.RespDecimal) != 0

    def setOvertempLimit(self, limit):
        """Set the temperature at which the firmware will shut down the lamp and return the new
        limit.
        """
        return self.execCommand('ot ' + str(limit), PJCInterface.RespFloat)

    def getOvertempLimit(self):
        """Get the temperature at which the firmware will shut the lamp down in order to protect
        projector components.
        """
        return self.execCommand('ot', PJCInterface.RespFloat)

    def setTargetTemperature(self, target):
        """Set the temperature the firmware will attempt to maintain at the thermistor.  The fan
        speed is adjusted to keep this temperature.
        """
        return self.execCommand('tt ' + str(target), PJCInterface.RespFloat)

    def getTargetTemperature(self):
        """Get the temperature the firmware will attempt to maintain at the thermistor.  The fan
        speed is adjusted to keep this temperature.
        """
        return self.execCommand('tt', PJCInterface.RespFloat)

    def setFanOffPoint(self, fanoff):
        """Set the point, relative to the ambient temperature, at which the firmware will turn the
        fan off.  This only takes effect if the lamp is off; the fan always runs while the lamp is
        on.
        """
        return self.execCommand('ft ' + str(fanoff), PJCInterface.RespFloat)

    def getFanOffPoint(self):
        """Get the point, relative to the ambient temperature, at which the firmware will turn the
        fan off.  This only takes effect if the lamp is off; the fan always runs while the lamp is
        on.
        """
        return self.execCommand('ft', PJCInterface.RespFloat)

    def setMinDutyCycle(self, mindc):
        """Set the minimum duty cycle in percent for the PWM controlling the fan speed and return
        the new duty cycle.
        """
        return self.execCommand('dcl ' + str(mindc), PJCInterface.RespFloat)

    def getMinDutyCycle(self):
        """Get the minimum duty cycle in percent for the PWM controlling the fan speed.
        """
        return self.execCommand('dcl', PJCInterface.RespFloat)

    def getCurrentDutyCycle(self):
        """Get the current duty cycle in percent.  The high this value, the faster the fan speed.
        """
        return self.execCommand('dc', PJCInterface.RespFloat)

    def saveSettingsToEEPROM(self):
        """Save application settings to the onboard EEPROM and return True if successful or False
        otherwise.
        """
        return self.execCommand('sv', PJCInterface.RespDecimal) != 0

    def getMostRecentError(self, clear):
        """Get the most recent error from the device as a hex value.  If 'clear' is True, then the
        error will be cleared so that future reads do not return the same error.
        """
        return self.execCommand('er ' + str(int(clear)), PJCInterface.RespHex)
