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

class TestConfigDBSS(unittest.TestCase):
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

    def test_dbss_good(self):
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123

            general:
                dc_files:
                    - %r

            roles:
                - type: dbss
                  database: 1200
                  ranges:
                      - min: 9000
                        max: 9999
            """ % test_dc
        self.assertEquals(self.run_test(config), TERMINATED)

    def test_dbss_invalid_attr(self):
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123

            roles:
                - type: dbss
                  pewpew: "q.q"
            """
        self.assertEquals(self.run_test(config), EXITED)

    def test_dbss_invalid_database(self):
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123

            roles:
                - type: dbss
                  database: 0
                  ranges:
                      - min: 9000
                        max: 9999
            """
        self.assertEquals(self.run_test(config), EXITED)

    def test_dbss_reserved_database(self):
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123

            roles:
                - type: dbss
                  database: 200
                  ranges:
                      - min: 9000
                        max: 9999
            """
        self.assertEquals(self.run_test(config), EXITED)

    def test_dbss_invalid_range(self):
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123

            roles:
                - type: dbss
                  database: 1200
                  ranges:
                      - min: 0
                        max: 9999
            """
        self.assertEquals(self.run_test(config), EXITED)

        config = """\
            messagedirector:
                bind: 127.0.0.1:57123

            roles:
                - type: dbss
                  database: 1200
                  ranges:
                      - min: 9000
                        max: 0
            """
        self.assertEquals(self.run_test(config), EXITED)

    def test_dbss_reserved_range(self):
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123

            roles:
                - type: dbss
                  database: 1200
                  ranges:
                      - min: 201
                        max: 9999
            """
        self.assertEquals(self.run_test(config), EXITED)

        config = """\
            messagedirector:
                bind: 127.0.0.1:57123

            roles:
                - type: dbss
                  database: 1200
                  ranges:
                      - min: 9000
                        max: 206
            """
        self.assertEquals(self.run_test(config), EXITED)

if __name__ == '__main__':
    unittest.main()
