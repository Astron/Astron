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
        return self.process.returncode

class TestConfigStateServer(unittest.TestCase):
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

    def test_stateserver_good(self):
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123
            general:
                dc_files:
                    - %r
            roles:
                - type: stateserver
                  control: 100100
            """ % test_dc
        self.assertEquals(self.run_test(config), TERMINATED)

    def test_ss_invalid_attr(self):
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123

            roles:
                - type: stateserver
                  queque: "pu-pu"
            """
        self.assertEquals(self.run_test(config), EXITED)

    def test_ss_invalid_control(self):
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123

            roles:
                - type: stateserver
                  control: 0
            """
        self.assertEquals(self.run_test(config), EXITED)

    def test_ss_reserved_control(self):
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123

            roles:
                - type: stateserver
                  control: 100
            """
        self.assertEquals(self.run_test(config), EXITED)

if __name__ == '__main__':
    unittest.main()
