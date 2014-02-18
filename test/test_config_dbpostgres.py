#!/usr/bin/env python2
import unittest
import subprocess
import threading
import tempfile
import os
import shutil

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

class TestConfigDBPostgres(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cfg, cls.config_file = tempfile.mkstemp()
        os.close(cfg)

        cls.test_command = ConfigTest(cls.config_file)

        cls.yaml_dir = tempfile.mkdtemp()

    @classmethod
    def tearDownClass(cls):
        if cls.config_file is not None:
            os.remove(cls.config_file)
        if cls.yaml_dir is not None:
            shutil.rmtree(cls.yaml_dir)


    @classmethod
    def write_config(cls, config):
        f = open(cls.config_file, "w")
        f.write(config)
        f.close()

    @classmethod
    def run_test(cls, config, timeout = 2):
        cls.write_config(config)
        return cls.test_command.run(timeout)

    def test_postgres_good(self):
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123

            general:
                dc_files:
                    - %r

            roles:
                - type: database
                  control: 75757
                  broadcast: true
                  generate:
                    min: 1000000
                    max: 1000010
                  backend:
                    type: postgresql
                    database: astron_test
            """ % test_dc
        self.assertEquals(self.run_test(config), TERMINATED)

    def test_postgres_reserved_control(self):
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123
            general:
                dc_files:
                    - %r
            roles:
                - type: database
                  control: 777
                  generate:
                    min: 1000000
                    max: 1000010
                  backend:
                    type: postgresql
                    database: astron_test
            """ % test_dc
        self.assertEquals(self.run_test(config), EXITED)

    def test_postgres_invalid_generate(self):
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123
            general:
                dc_files:
                    - %r
            roles:
                - type: database
                  control: 75757
                  generate:
                    min: 0
                    max: 1000010
                  backend:
                    type: postgresql
                    database: astron_test
            """ % test_dc
        self.assertEquals(self.run_test(config), EXITED)

        config = """\
            messagedirector:
                bind: 127.0.0.1:57123
            general:
                dc_files:
                    - %r
            roles:
                - type: database
                  control: 75757
                  generate:
                    min: 1000000
                    max: 0
                  backend:
                    type: postgresql
                    database: astron_test
            """ % test_dc
        self.assertEquals(self.run_test(config), EXITED)

    def test_postgres_reserved_generate(self):
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123
            general:
                dc_files:
                    - %r
            roles:
                - type: database
                  control: 75757
                  generate:
                    min: 444
                    max: 1000010
                  backend:
                    type: postgresql
                    database: astron_test
            """ % test_dc
        self.assertEquals(self.run_test(config), EXITED)

        config = """\
            messagedirector:
                bind: 127.0.0.1:57123
            general:
                dc_files:
                    - %r
            roles:
                - type: database
                  control: 75757
                  generate:
                    min: 1000000
                    max: 555
                  backend:
                    type: postgresql
                    database: astron_test
            """ % test_dc
        self.assertEquals(self.run_test(config), EXITED)

    def test_postgres_boolean_broadcast(self):
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123

            general:
                dc_files:
                    - %r

            roles:
                - type: database
                  control: 75757
                  broadcast: TRUE
                  generate:
                    min: 1000000
                    max: 1000010
                  backend:
                    type: postgresql
                    database: astron_test
            """ % test_dc
        self.assertEquals(self.run_test(config), TERMINATED)

        config = """\
            messagedirector:
                bind: 127.0.0.1:57123

            general:
                dc_files:
                    - %r

            roles:
                - type: database
                  control: 75757
                  broadcast: False
                  generate:
                    min: 1000000
                    max: 1000010
                  backend:
                    type: postgresql
                    database: astron_test
            """ % test_dc
        self.assertEquals(self.run_test(config), TERMINATED)

        config = """\
            messagedirector:
                bind: 127.0.0.1:57123

            general:
                dc_files:
                    - %r

            roles:
                - type: database
                  control: 75757
                  broadcast: 15012
                  generate:
                    min: 1000000
                    max: 1000010
                  backend:
                    type: postgresql
                    database: astron_test
            """ % test_dc
        self.assertEquals(self.run_test(config), EXITED)

if __name__ == '__main__':
    unittest.main()
