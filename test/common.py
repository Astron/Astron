import subprocess
import tempfile
import struct
import socket
import time
import os

__all__ = ['Daemon', 'Datagram', 'DatagramIterator', 'MDConnection']

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
    # Reserved Values
    'INVALID_CHANNEL': 0,
    'INVALID_DO_ID': 0,
    'INVALID_MSG_TYPE': 0,

    # Control Channels
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
    'STATESERVER_OBJECT_QUERY_ALL_RESP': 2030,
    'STATESERVER_SHARD_RESET': 2061,
    'STATESERVER_OBJECT_ENTERZONE_WITH_REQUIRED': 2065,
    'STATESERVER_OBJECT_ENTERZONE_WITH_REQUIRED_OTHER': 2066,
    'STATESERVER_OBJECT_ENTER_AI_RECV': 2067,
    'STATESERVER_OBJECT_LEAVING_AI_INTEREST': 2033,
    'STATESERVER_OBJECT_QUERY_MANAGING_AI': 2083,
    'STATESERVER_OBJECT_NOTIFY_MANAGING_AI': 2047,


    # Database Server
    'DBSERVER_CREATE_STORED_OBJECT': 1003,
    'DBSERVER_CREATE_STORED_OBJECT_RESP': 1004,
    'DBSERVER_DELETE_STORED_OBJECT': 1008,
    'DBSERVER_DELETE_QUERY': 1010,
    'DBSERVER_SELECT_STORED_OBJECT': 1012,
    'DBSERVER_SELECT_STORED_OBJECT_RESP': 1013,
    'DBSERVER_SELECT_STORED_OBJECT_ALL': 1020,
    'DBSERVER_SELECT_STORED_OBJECT_ALL_RESP': 1021,
    'DBSERVER_SELECT_QUERY': 1016,
    'DBSERVER_SELECT_QUERY_RESP': 1017,
    'DBSERVER_UPDATE_STORED_OBJECT': 1014,
    'DBSERVER_UPDATE_STORED_OBJECT_IF_EQUALS': 1024,
    'DBSERVER_UPDATE_STORED_OBJECT_IF_EQUALS_RESP': 1025,
    'DBSERVER_UPDATE_QUERY': 1018,
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

class DatagramIterator(object):
    def __init__(self, datagram, offset = 0):
        self._datagram = datagram
        self._data = datagram.get_data()
        self._offset = offset

    def matches_header(self, recipients, sender, msgtype, remaining=-1):
        self.seek(0)
        channels = [i for i, j in zip(self._datagram.get_channels(), recipients) if i == j]
        if len(channels) != len(recipients):
            return False

        self.seek(8*ord(self._data[0])+1)
        if sender != self.read_uint64():
            return False

        if msgtype != self.read_uint16():
            return False

        if remaining != -1 and remaining != len(self.data) - offset_:
            return False

        return True

    def read_uint8(self):
        self._offset += 2
        if self._offset > len(self.data):
            raise EOFError('End of Datagram')

        return struct.unpack("<B", self._data[self._offset-2:self._offset])

    def read_uint16(self):
        self._offset += 4
        if self._offset > len(self.data):
            raise EOFError('End of Datagram')

        return struct.unpack("<H", self._data[self._offset-4:self._offset])

    def read_uint64(self):
        self._offset += 8
        if self._offset > len(self.data):
            raise EOFError('End of Datagram')

        return struct.unpack("<H", self._data[self._offset-8:self._offset])

    def seek(self, offset):
        self._offset = offset

    def tell(self):
        return self._offset

class MDConnection(object):
    def __init__(self, sock):
        self.s = sock
        self.s.settimeout(0.1)

    def send(self, datagram):
        data = datagram.get_data()
        msg = struct.pack('<H', len(data)) + data
        self.s.send(msg)

    def recv(self):
        dg = self._read()
        if dg is None:
            raise EOFError('No message recieved')
        return Datagram(dg)

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
