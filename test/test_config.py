#!/usr/bin/env python2
import unittest
from common.unittests import ConfigTest
from common.dcfile import *
import socket

def test_ipv6():
    try:
        s = socket.socket(socket.AF_INET6, socket.SOCK_STREAM)
        s.bind(('::1', 0))
    except socket.error:
        return False
    else:
        return True

class TestConfigCore(ConfigTest):
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
                bind: 127.0.0.1:57123
                threaded: true
            """ % test_dc
        self.assertEquals(self.checkConfig(config), 'Valid')

    def test_without_threading(self):
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
                bind: 127.0.0.1:57123
                threaded: false
            """ % test_dc
        self.assertEquals(self.checkConfig(config), 'Valid')

    def test_roles_missing_type(self):
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123
            roles:
                - qux: bar
                - bleem: baz"""
        self.assertEquals(self.checkConfig(config), 'Invalid')

    def test_roles_invalid_type(self):
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123
            roles:
                - type: foo
            """
        self.assertEquals(self.checkConfig(config), 'Invalid')

    def test_root_is_not_map(self):
        config = """\
            - messagedirector:
                  bind: 127.0.0.1:57123
            - roles:
                  - type: foo
            """
        self.assertEquals(self.checkConfig(config), 'Invalid')

    def test_general_is_not_map(self):
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123
            general:
                - eventlogger: 127.0.0.1:9090
                - dc_files:
                      - %r
            """ % test_dc
        self.assertEquals(self.checkConfig(config), 'Invalid')

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
                anonymous: false
            """
        self.assertEquals(self.checkConfig(config), 'Invalid')

    def test_uberdogs_invalid_id(self):
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123

            uberdogs:
                - id: 0
                  class: UberDog1
            """
        self.assertEquals(self.checkConfig(config), 'Invalid')

    def test_uberdogs_reserved_id(self):
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123

            uberdogs:
                - id: 165
                  class: UberDog2
            """
        self.assertEquals(self.checkConfig(config), 'Invalid')

    def test_core_address_hosts(self):
        config = """\
            messagedirector:
                bind: 127.0.0.1:57123
            general:
                eventlogger: 127.0.0.1:9090
            """
        self.assertEquals(self.checkConfig(config), 'Valid')

        # ipv6 test disabled because message director can't accept them yet, and causes a crash
        #config = """\
        #    messagedirector:
        #        bind: "::1:57123"
        #    general:
        #        eventlogger: "::1:9090"
        #    """
        #self.assertEquals(self.checkConfig(config), 'Valid')

        config = """\
            messagedirector:
                bind: 127.0.0:20
            """
        self.assertEquals(self.checkConfig(config), 'Invalid')

        # Windows actually supports incomplete addresses like this,
        #    since its system specific, we'll just let the OS decide whether
        #    it can resolve this type of address and not assert that it is invalid.
        #config = """\
        #    messagedirector:
        #        bind: 127.0.0.1:57123
        #    general:
        #        eventlogger: 0.0.1:9090
        #    """
        #self.assertEquals(self.checkConfig(config), 'Invalid')

        config = """\
            messagedirector:
                bind: foobar:20
            """
        self.assertEquals(self.checkConfig(config, 10), 'Invalid')

        config = """\
            messagedirector:
                bind: 127.0.0.1:57123
            general:
                eventlogger: ble3.3.3.3:20
            """
        self.assertEquals(self.checkConfig(config, 10), 'Invalid')

        config = """\
            messagedirector:
                bind: 10.0.0.0foobar
            """
        self.assertEquals(self.checkConfig(config, 10), 'Invalid')

        config = """\
            messagedirector:
                bind: 10.0.blam.20
            """
        self.assertEquals(self.checkConfig(config, 10), 'Invalid')

        config = """\
            messagedirector:
                bind: pizza-pie
            """
        self.assertEquals(self.checkConfig(config, 10), 'Invalid')

        config = """\
            messagedirector:
                bind: "127:0:0:1:57123"
            """
        self.assertEquals(self.checkConfig(config), 'Invalid')

        config = """\
            messagedirector:
                bind: "127-0-0-1:57123"
            """
        self.assertEquals(self.checkConfig(config, 10), 'Invalid')

        config = """\
            messagedirector:
                bind: "99-0-0-1:57123"
            """
        self.assertEquals(self.checkConfig(config, 10), 'Invalid')

        config = """\
            messagedirector:
                bind: "99.1:57123"
            """
        self.assertEquals(self.checkConfig(config), 'Invalid')

        # And now to test some hostname cases...
        config = """\
            messagedirector:
                bind: "localhost:57123"
            """
        self.assertEquals(self.checkConfig(config), 'Valid')

        config = """\
            messagedirector:
                bind: "google.com:57123"
            """
        self.assertEquals(self.checkConfig(config), 'Invalid')

        config = """\
            messagedirector:
                bind: "nxdomain.doesntexist.foo:57123"
            """
        self.assertEquals(self.checkConfig(config, 10), 'Invalid')

        config = """\
            messagedirector:
                bind: "bad~characters:57123"
            """
        self.assertEquals(self.checkConfig(config), 'Invalid')

        config = """\
            messagedirector:
                bind: "more--bad.characters..-:57123"
            """
        self.assertEquals(self.checkConfig(config), 'Invalid')

        # Some IPv6 cases...

        # Note: These only work if the test runner has IPv6 support. Let's
        # check:

        has_ipv6 = test_ipv6()

        config = """\
            messagedirector:
                bind: "::1"
            """
        if has_ipv6: self.assertEquals(self.checkConfig(config), 'Valid')

        config = """\
            messagedirector:
                bind: "0:0:0:0:0:0:0:1"
            """
        if has_ipv6: self.assertEquals(self.checkConfig(config), 'Valid')

        config = """\
            messagedirector:
                bind: "[0:0::1]"
            """
        if has_ipv6: self.assertEquals(self.checkConfig(config), 'Valid')

        config = """\
            messagedirector:
                bind: "[0:0::1]:57123"
            """
        if has_ipv6: self.assertEquals(self.checkConfig(config), 'Valid')

        config = """\
            messagedirector:
                bind: "[0:0::1:57123"
            """
        self.assertEquals(self.checkConfig(config), 'Invalid')

        config = """\
            messagedirector:
                bind: "0:0::1]:57123"
            """
        self.assertEquals(self.checkConfig(config), 'Invalid')

        config = """\
            messagedirector:
                bind: "0:0::1:57123"
            """
        self.assertEquals(self.checkConfig(config), 'Invalid')

if __name__ == '__main__':
    unittest.main()
