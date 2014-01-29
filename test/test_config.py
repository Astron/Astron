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

class TestConfigCore(unittest.TestCase):
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

    def test_core_good(self):
        config = """\
            daemon:
                name: Core Message Director
                url: http://123.45.67.89/coremd/

            general:
                eventlogger: 127.0.0.1:9090
                dc_files:
                    - %r

            uberdogs:
                - id: 1234
                  class: UberDog1
                  anonymous: true

                - id: 1235
                  class: UberDog2
                  anonymous: false

            messagedirector:
                bind: 127.0.0.1:57123""" % test_dc
        self.assertEquals(self.run_test(config), TERMINATED)

    def test_roles_missing_type(self):
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123
            roles:
                - qux: bar
                - bleem: baz"""
        self.assertEquals(self.run_test(config), EXITED)

    def test_roles_invalid_type(self):
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123
            roles:
                - type: foo"""
        self.assertEquals(self.run_test(config), EXITED)

    def test_root_is_not_map(self):
        config = """\
            - messagedirector:
                  bind: 127.0.0.1:57123
            - roles:
                  - type: foo"""
        self.assertEquals(self.run_test(config), EXITED)

    def test_general_is_not_map(self):
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123
            general:
                - eventlogger: 127.0.0.1:9090
                - dc_files:
                      - %r""" % test_dc
        self.assertEquals(self.run_test(config), EXITED)

    def test_uberdogs_is_not_list(self):
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123
            uberdogs:
                id: 1234
                class: UberDog1
                anonymous: true

                id: 1235
                class: UberDog2
                anonymous: false"""
        self.assertEquals(self.run_test(config), EXITED)

if __name__ == '__main__':
    unittest.main()
