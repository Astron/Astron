#!/usr/bin/env python2
import unittest
from common.unittests import ConfigTest
from common.dcfile import *

class TestConfigClientAgent(ConfigTest):
    def test_clientagent_good(self):
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123

            general:
                dc_files:
                    - %r

            uberdogs:
                - id: 1234
                  class: UberDog1
                  anonymous: true

                - id: 1235
                  class: UberDog2
                  anonymous: false

            roles:
                - type: clientagent
                  bind: 127.0.0.1:57128
                  version: "Sword Art Online v5.1"
                  channels:
                      min: 3100
                      max: 3999
                  client:
                      relocate: true
                      add_interest: enabled

                - type: clientagent
                  bind: 127.0.0.1:57135
                  version: "Sword Art Online v5.1"
                  client:
                      type: libastron
                      add_interest: disabled
                      write_buffer_size: 262144
                      write_timeout_ms: 20
                  channels:
                      min: 110600
                      max: 110699

                - type: clientagent
                  bind: 127.0.0.1:57144
                  version: "Sword Art Online v5.1"
                  client:
                      type: libastron
                      add_interest: visible
                      write_buffer_size: 0
                      write_timeout_ms: 0
                  channels:
                      min: 220600
                      max: 220699
            """ % test_dc
        self.assertEquals(self.checkConfig(config), 'Valid')

    def test_ca_invalid_attr(self):
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123

            roles:
                - type: clientagent
                  bind: 127.0.0.1:57128
                  version: "Sword Art Online v5.1"
                  channels:
                      min: 3100
                      max: 3999
                  weeuuweeeuu: sirens!
            """
        self.assertEquals(self.checkConfig(config), 'Invalid')

    def test_ca_invalid_channels(self):
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123

            roles:
                - type: clientagent
                  bind: 127.0.0.1:57128
                  version: "Sword Art Online v5.1"
                  channels:
                      min: 0
                      max: 3999
            """
        self.assertEquals(self.checkConfig(config), 'Invalid')

        config = """\
            messagedirector:
                bind: 127.0.0.1:57123

            roles:
                - type: clientagent
                  bind: 127.0.0.1:57128
                  version: "Sword Art Online v5.1"
                  channels:
                      min: 3100
                      max: 0
            """
        self.assertEquals(self.checkConfig(config), 'Invalid')

    def test_ca_reserved_channels(self):
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123

            roles:
                - type: clientagent
                  bind: 127.0.0.1:57128
                  version: "Sword Art Online v5.1"
                  channels:
                      min: 100
                      max: 3999
            """
        self.assertEquals(self.checkConfig(config), 'Invalid')

        config = """\
            messagedirector:
                bind: 127.0.0.1:57123

            roles:
                - type: clientagent
                  bind: 127.0.0.1:57128
                  version: "Sword Art Online v5.1"
                  channels:
                      min: 3100
                      max: 999
            """
        self.assertEquals(self.checkConfig(config), 'Invalid')

    def test_ca_client_type_typo(self):
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123

            roles:
                - type: clientagent
                  bind: 127.0.0.1:57128
                  version: "Sword Art Online v5.1"
                  client:
                      type: astron
                  channels:
                      min: 3100
                      max: 3999
            """
        self.assertEquals(self.checkConfig(config), 'Invalid')

    def test_ca_bind_address(self):
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123

            roles:
                - type: clientagent
                  bind: pizza:2314
                  version: "Sword Art Online v5.1"
                  channels:
                      min: 3100
                      max: 3999
            """
        self.assertEquals(self.checkConfig(config, 10), 'Invalid')

        # ipv6 test disabled because client agent can't accept them yet, and causes a crash
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123

            roles:
                - type: clientagent
                  bind: ::1:2314
                  version: "Sword Art Online v5.1"
                  channels:
                      min: 3100
                      max: 3999
            """
        #self.assertEquals(self.checkConfig(config), 'Valid')

if __name__ == '__main__':
    unittest.main()
