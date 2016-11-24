#!/usr/bin/env python2
import unittest
from common.unittests import ConfigTest
from common.dcfile import *

class TestConfigEventLogger(ConfigTest):
    def test_eventlogger_good(self):
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123

            roles:
                - type: eventlogger
                  bind: 0.0.0.0:9090
                  output: /var/log/astron/eventlogger/el-%Y-%m-%d-%H-%M-%S.log
                  rotate_interval: 1d
            """
        self.assertEquals(self.checkConfig(config), 'Valid')

    def test_eventlogger_bind_address(self):
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123

            roles:
                - type: eventlogger
                  bind: pizza:2314
                  output: /var/log/astron/eventlogger/el-%Y-%m-%d-%H-%M-%S.log
                  rotate_interval: 1d
            """
        self.assertEquals(self.checkConfig(config, 10), 'Invalid')

        # ipv6 test disabled because client agent can't accept them yet, and causes a crash
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123

            roles:
                - type: eventlogger
                  bind: ::1:2314
                  output: /var/log/astron/eventlogger/el-%Y-%m-%d-%H-%M-%S.log
                  rotate_interval: 1d
            """
        #self.assertEquals(self.checkConfig(config), 'Valid')

if __name__ == '__main__':
    unittest.main()
