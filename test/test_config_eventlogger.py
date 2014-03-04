#!/usr/bin/env python2
import unittest
import subprocess
import threading
import tempfile
import os

from testdc import *

DAEMON_PATH = './astrond'
TERMINATED = -15
EXITED = 1

class ConfigTest(object):
    def __init__(self, config):
        self.config = config
        self.process = None

    def run(self, timeout):
        def target():
            self.process = subprocess.Popen([DAEMON_PATH, self.config])
            self.process.communicate()

        thread = threading.Thread(target=target)
        thread.start()

        thread.join(timeout)
        if thread.is_alive():
            self.process.terminate()
            thread.join()
            return TERMINATED
        return EXITED

class TestConfigEventLogger(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cfg, cls.config_file = tempfile.mkstemp()
        os.close(cfg)

        cls.test_command = ConfigTest(cls.config_file)

    @classmethod
    def tearDownClass(cls):
        if cls.config_file is not None:
            os.remove(cls.config_file)

    @classmethod
    def write_config(cls, config):
        f = open(cls.config_file, "w")
        f.write(config)
        f.close()

    @classmethod
    def run_test(cls, config, timeout = 2):
        cls.write_config(config)
        return cls.test_command.run(timeout)

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
        self.assertEquals(self.run_test(config), TERMINATED)

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
        self.assertEquals(self.run_test(config), EXITED)

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
        #self.assertEquals(self.run_test(config), TERMINATED)

if __name__ == '__main__':
    unittest.main()
