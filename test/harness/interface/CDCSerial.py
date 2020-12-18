# Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
# For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md
import time
from random import randrange

import serial
import json
import logging
from enum import Enum

from harness.interface.defs import endpoint, method, status
from harness.interface.error import TestError, Error

log = logging.getLogger(__name__)


class Keytype(Enum):
    long_press = 0
    short_press = 1


class CDCSerial:
    def __init__(self, port_name, timeout=10):
        self.timeout = timeout
        self.body = ""
        self.header_length = 10
        while timeout != 0:
            try:
                self.serial = serial.Serial(port_name, baudrate=115200, timeout=10)
                self.serial.flushInput()
                log.info("port opened!")
                break
            except (FileNotFoundError, serial.serialutil.SerialException) as err:
                log.error("can't open {}, retrying...".format(port_name))
                time.sleep(1)
                self.timeout = self.timeout - 1
                if self.timeout == 0:
                    log.error("uart {} not found - probably OS did not boot".format(port_name))
                    raise TestError(Error.PORT_NOT_FOUND)

    def __del__(self):
        try:
            self.serial.close()
        except (serial.serialutil.SerialException, AttributeError):
            pass

    def __wrap_message(self, body):
        msg = {
            "endpoint": endpoint["developerMode"],
            "method": method["put"],
            "uuid": randrange(1,100),
            "body": body
        }
        return msg

    def get_serial(self):
        return self.serial

    def __build_message(self, json_data):
        json_dump = json.dumps(json_data)
        return "#%09d%s" % (len(json_dump), json_dump)

    def read(self, length):
        header = self.readRaw(length)
        payload_length = int(header[1:])
        result = self.readRaw(payload_length)
        log.info(f"received length: {len(result)}, payload length:{payload_length}")
        return result

    def readRaw(self, length):
        return self.serial.read(length).decode()

    def write(self, msg, timeout=30):
        message = self.__build_message(msg)
        self.writeRaw(message)
        result = self.read(self.header_length)
        return json.loads(result)

    def writeRaw(self, message, timeout=30):
        log.info(message)
        self.serial.write(message.encode())
        self.serial.timeout = timeout

    def send_key(self, key_code, key_type=Keytype.short_press, wait=10):
        if key_type is Keytype.long_press:
            body = {"keyPressed": key_code, "state": 4}
        else:
            body = {"keyPressed": key_code, "state": 2}
        ret = self.write(self.__wrap_message(body), wait)
        time.sleep(0.3)
        return ret

    def enable_echo_mode(self):
        echoOnCmd = "UsbCdcEcho=ON"
        self.writeRaw(echoOnCmd)
        result = self.readRaw(len(echoOnCmd))
        log.info(f"received length: {len(result)}, result:{result}")
        ret = (result == echoOnCmd)
        return ret

    def disable_echo_mode(self):
        echoOffCmd = "UsbCdcEcho=OFF"
        self.writeRaw(echoOffCmd)
        result = self.readRaw(len(echoOffCmd))
        log.info(f"received length: {len(result)}, result:{result}")
        ret = (result == echoOffCmd)
        return ret

    def send_at(self, at_command, wait=10):
        body = {
            "AT": at_command + "\r"
        }

        ret = self.write(self.__wrap_message(body), wait)
        print(ret)
        return ret["body"]["ATResponse"]

    def get_window_name(self):
        body = {
            "focus": True
        }

        ret = self.write(self.__wrap_message(body))
        return ret["body"]["focus"]

    def is_phone_locked(self):
        body = {
            "isLocked": True
        }

        ret = self.write(self.__wrap_message(body))
        return ret["body"]["isLocked"]
