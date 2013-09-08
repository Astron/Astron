import subprocess
import tempfile
import struct
import socket
import time
import os

__all__ = ['Daemon', 'Datagram', 'MDConnection']

class Daemon(object):
    DAEMON_PATH = './openotpd'

    def __init__(self, config):
        self.config = config

        self.daemon = None
        self.config_file = None

    def start(self):
        cfg, self.config_file = tempfile.mkstemp()
        os.write(cfg, self.config)
        os.close(cfg)

        self.daemon = subprocess.Popen([self.DAEMON_PATH,
                                        '-config', self.config_file])

        time.sleep(1.0) # Allow some time for daemon to initialize...

    def stop(self):
        self.daemon.kill()
        os.remove(self.config_file)

DATATYPES = {
    'int8': '<b',
    'uint8': '<B',
    'int16': '<h',
    'uint16': '<H',
    'int32': '<i',
    'uint32': '<I',
    'int64': '<q',
    'uint64': '<Q',
}

CONSTANTS = {
    'CONTROL_CHANNEL': 4001,
    'CONTROL_ADD_CHANNEL': 2001,
    'CONTROL_REMOVE_CHANNEL': 2002,

    'CONTROL_ADD_RANGE': 2008,
    'CONTROL_REMOVE_RANGE': 2009,

    'CONTROL_ADD_POST_REMOVE': 2010,
    'CONTROL_CLEAR_POST_REMOVE': 2011,


    # State Server
    'STATESERVER_OBJECT_GENERATE_WITH_REQUIRED': 2001,
    'STATESERVER_OBJECT_GENERATE_WITH_REQUIRED_OTHER': 2003,
    'STATESERVER_OBJECT_UPDATE_FIELD': 2004,
    'STATESERVER_OBJECT_UPDATE_FIELD_MULTIPLE': 2005,
    'STATESERVER_OBJECT_DELETE_RAM': 2007,
    'STATESERVER_OBJECT_SET_ZONE': 2008,
    'STATESERVER_OBJECT_CHANGE_ZONE': 2009,
    'STATESERVER_OBJECT_LOCATE': 2022,
    'STATESERVER_OBJECT_LOCATE_RESP': 2023,
    'STATESERVER_OBJECT_SET_AI_CHANNEL': 2045,
    'STATESERVER_OBJECT_QUERY_ALL': 2020,
    'STATESERVER_OBJECT_QUERY_FIELD': 2024,
    'STATESERVER_OBJECT_QUERY_FIELD_RESP': 2062,
    'STATESERVER_OBJECT_QUERY_FIELDS': 2080,
    'STATESERVER_OBJECT_QUERY_FIELDS_RESP': 2081,
    'STATESERVER_OBJECT_QUERY_ALL_RESP': 2030,
    'STATESERVER_SHARD_RESET': 2061,
    'STATESERVER_OBJECT_ENTERZONE_WITH_REQUIRED': 2065,
    'STATESERVER_OBJECT_ENTERZONE_WITH_REQUIRED_OTHER': 2066,
    'STATESERVER_OBJECT_ENTER_AI_RECV': 2067,
    'STATESERVER_OBJECT_SET_OWNER_RECV': 2070,
    'STATESERVER_OBJECT_CHANGE_OWNER_RECV': 2069,
    'STATESERVER_OBJECT_ENTER_OWNER_RECV': 2068,
    'STATESERVER_OBJECT_LEAVING_AI_INTEREST': 2033,
    'STATESERVER_OBJECT_QUERY_MANAGING_AI': 2083,
    'STATESERVER_OBJECT_NOTIFY_MANAGING_AI': 2047,
}

locals().update(CONSTANTS)
__all__.extend(CONSTANTS.keys())

class Datagram(object):
    def __init__(self, data=b''):
        self._data = data

        def make_adder(v):
            def adder(data):
                self.add_raw(struct.pack(v, data))
            return adder

        for k,v in DATATYPES.items():
            adder = make_adder(v)
            setattr(self, 'add_' + k, adder)

    def add_raw(self, data):
        self._data += data

    def add_string(self, string):
        self.add_uint16(len(string))
        self.add_raw(string)

    def get_data(self):
        return self._data

    def get_payload(self):
        return self._data[8*ord(self._data[0])+1:]

    def get_channels(self):
        return set(struct.unpack('<x' + 'Q'*ord(self._data[0]),
                                 self._data[:8*ord(self._data[0])+1]))

    def is_subset_of(self, other):
        return self.get_payload() == other.get_payload() and \
               self.get_channels() <= other.get_channels()

    # Common datagram type helpers:
    @classmethod
    def create(cls, recipients, sender, msgtype):
        dg = cls()
        dg.add_uint8(len(recipients))
        for recipient in recipients: dg.add_uint64(recipient)
        dg.add_uint64(sender)
        dg.add_uint16(msgtype)
        return dg

    @classmethod
    def create_control(cls):
        dg = cls()
        dg.add_uint8(1)
        dg.add_uint64(CONTROL_CHANNEL)
        return dg

    @classmethod
    def create_add_channel(cls, channel):
        dg = cls.create_control()
        dg.add_uint16(CONTROL_ADD_CHANNEL)
        dg.add_uint64(channel)
        return dg

    @classmethod
    def create_remove_channel(cls, channel):
        dg = cls.create_control()
        dg.add_uint16(CONTROL_REMOVE_CHANNEL)
        dg.add_uint64(channel)
        return dg

    @classmethod
    def create_add_range(cls, upper, lower):
        dg = cls.create_control()
        dg.add_uint16(CONTROL_ADD_RANGE)
        dg.add_uint64(upper)
        dg.add_uint64(lower)
        return dg

    @classmethod
    def create_remove_range(cls, upper, lower):
        dg = cls.create_control()
        dg.add_uint16(CONTROL_REMOVE_RANGE)
        dg.add_uint64(upper)
        dg.add_uint64(lower)
        return dg

    @classmethod
    def create_add_post_remove(cls, datagram):
        dg = cls.create_control()
        dg.add_uint16(CONTROL_ADD_POST_REMOVE)
        dg.add_string(datagram.get_data())
        return dg

    @classmethod
    def create_clear_post_remove(cls):
        dg = cls.create_control()
        dg.add_uint16(CONTROL_CLEAR_POST_REMOVE)
        return dg

class MDConnection(object):
    def __init__(self, sock):
        self.s = sock
        self.s.settimeout(0.1)

    def send(self, datagram):
        data = datagram.get_data()
        msg = struct.pack('<H', len(data)) + data
        self.s.send(msg)

    def _read(self):
        try:
            length = 2
            result = ''
            while len(result) < length:
                data = self.s.recv(length - len(result))
                if data == '':
                    raise EOFError('Remote socket closed connection')
                result += data
            length = struct.unpack('<H', result)[0]
        except socket.error:
            return None

        result = ''
        while len(result) < length:
            data = self.s.recv(length - len(result))
            if data == '':
                    raise EOFError('Remote socket closed connection')
            result += data

        return result

    def close(self):
        self.s.close()

    def flush(self):
        while self._read(): pass

    def expect(self, datagram):
        return self.expect_multi([datagram], only=True)

    def expect_multi(self, datagrams, only=False):
        datagrams = list(datagrams) # We're going to be doing datagrams.remove()

        while datagrams:
            dg = self._read()
            if dg is None: return False # Augh, we didn't see all the dgs yet!
            dg = Datagram(dg)

            for datagram in datagrams:
                if datagram.is_subset_of(dg):
                    datagrams.remove(datagram)
                    break
            else:
                if only:
                    return False

        return True


    def expect_none(self):
        return self._read() == None
