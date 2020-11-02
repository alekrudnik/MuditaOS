# Copyright (c) 2017-2020, Mudita Sp. z.o.o. All rights reserved.
# For licensing, see https://github.com/mudita/MuditaOS/LICENSE.md

from interface.defs import *
from test import *


class FactoryResetTest:
    def __init__(self, serial):
        self.serial = serial.get_serial()

    def run(self):
        msg, result_msg = prepare_message(endpoint["factory"], method["post"], status["OK"], {"factoryRequest": True},
                                          {'factoryRequest': True})
        test = Test(self.serial, msg, result_msg)
        ret, result = test.execute()
        return ret
