__description__ = \
"""
Control an automatic door via an arudino interface.  The arduino is expected to
have an "open" sensor, a "closed" sensor, and an ambient light sensor.
"""
__author__ = "Michael J. Harms"
__date__ = "2017-09-23"

import PyCmdMessenger

import serial

import os, time


class DoorException(Exception):
    """
    General error class for this module.
    """

    pass

class Door:
    """
    Automatic door, controlled by arduino.
    """

    def __init__(self,
                 device_name,
                 device_tty=None,
                 door_move_time=5):
        """
        device_name: name that the device will return if pinged by "who_are_you"
                   arduino sketch
        device_tty: serial device for arduino.  if None, look for the device under
                    /dev/ttyA* devices
        door_move_time: time to wait for door before checking on it (s)
        """

        self._COMMANDS = (("who_are_you",""),
                          ("query",""),
                          ("open_door",""),
                          ("close_door",""),
                          ("who_are_you_return","s"),
                          ("query_return","iii"),
                          ("communication_error","s"))
        self._BAUD_RATE = 9600                   

        self._device_name = device_name
        self._device_tty = device_tty
        self._door_move_time = door_move_time

        self._last_check = -1

        # Status that indicates whether the software device has actually found
        # any hardware.
        self._hardware_is_found = False

        # Try to connect to specified device
        if self._device_tty is not None:

            try:
                self._arudino_raw_serial = PyCmdMessenger.ArduinoBoard(self._device_tty,
                                                                       baud_rate=self._BAUD_RATE)
                self._arduino_msg = PyCmdMessenger.CmdMessenger(self._arduino_raw_serial,
                                                                self._COMMANDS)
                self._hardware_is_found = True

            except:
                pass

        # Or look for the device
        else:
            self._find_serial()

        if not self._hardware_is_found:
            err = "Could not connect to door hardware.\n"
            raise DoorException(err)

        # Find current state of the system
        self._query()

    def _find_serial(self):
        """
        Search through attached serial devices until one reports the specified
        self._device_name when probed by "who_are_you".
        """

        # if there is already a serial connection, move on
        if self._hardware_is_found:
            return

        tty_devices = [d for d in os.listdir("/dev") if d.startswith("ttyA")]

        for d in tty_devices:

            try:
                tmp_tty = os.path.join("/dev",d)
                a = PyCmdMessenger.ArduinoBoard(tmp_tty,self._BAUD_RATE)
                cmd = PyCmdMessenger.CmdMessenger(a,self._COMMANDS)

                cmd.send("who_are_you")
                reply = cmd.receive()
                if reply != None:
                    if reply[0] == "who_are_you_return":
                        if reply[1][0] == self._device_name:
                            self._arduino_raw_serial = a
                            self._arduino_msg = cmd
                            self._device_tty = tmp_tty
                            self._hardware_is_found = True
                            break

            # something went wrong ... not a device we can use.
            except IndexError:
                pass

    def _query(self):
        """
        Return sensor status.
        """
       
        self._arduino_msg.send("query")
        result = self._arduino_msg.receive()

        if result[0] != "query_return":
            err = "door query failed.\n"
            raise DoorException(err)
        
 
        # do a check thing
        self._ambient_light = result[1][0]
        open_sensor = result[1][1]
        closed_sensor = result[1][2]
        
        # interpret result
        if open_sensor < 200 and closed_sensor > 900:
            self._door_state = "open"
        elif open_sensor > 900 and closed_sensor < 200: 
            self._door_state = "closed"
        else:
            self._door_state = "unknown"

        self._last_check = time.time()
 
    def open_door(self):
        """
        """
        # send open door command
        self._arduino_msg.send("open_door")
        time.sleep(self._door_move_time)

        # send status check and make sure it's open
        self._query()
        if self._door_state != "open":
            err = "Door is reading {} after open request.\n".format(self._door_state)
            raise DoorException(err)

    def close_door(self):
        """
        """
        # send close door command
        self._arduino_msg.send("close_door")
        time.sleep(self._door_move_time)

        # send forced status check and make sure it's open
        self._query()
        if self._door_state != "closed":
            err = "Door is reading {} after close request.\n".format(self._door_state)
            raise DoorException(err)

    @property
    def comm_state(self):
        """
        Return tuple.  First entry is True/False (indicating good comms or not).
        Second entry is string describing current state.
        """

        if not self._hardware_is_found:
            return False, "No hardware found."

        try:
            self._arduino_msg.send("who_are_you") 
            result = self._arduino_msg.receive()
            if result[0] != "who_are_you_return" or result[1][0] != self._device_name:
                err = "device name has changed to {}\n".format(result[1][0])
                return False, err
        except serial.serialutil.SerialException:
            err = "serial connection lost\n"
            return False, err

        return True, "Connected."

    @property
    def door_state(self):
        """
        Check current status and return "open", "closed", "unknown".  
        """

        self._query()
        
        return self._door_state 

    @property
    def ambient_light(self):
        """
        Return current light level.
        """

        self._query()

        return self._ambient_light
    
    @property
    def last_check(self):
        """
        Return last time system was checked.
        """

        return self._last_check
