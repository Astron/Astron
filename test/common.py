import subprocess
import tempfile
import struct
import socket
import time
import os

__all__ = ['Daemon', 'Datagram', 'DatagramIterator', 'MDConnection', 'ChannelConnection']

class Daemon(object):
    DAEMON_PATH = './astrond'

    def __init__(self, config):
        self.config = config

        self.daemon = None
        self.config_file = None

    def start(self):
        if 'MANUAL_LAUNCH_CONFIG' in os.environ:
            # User wants to manually launch their Astron daemon, so we'll write
            # out the config for them and prompt them to
            with open(os.environ['MANUAL_LAUNCH_CONFIG'], 'wb') as config:
                config.write(self.config)

            raw_input('Waiting for manual launch; press enter when ready...')

            return # Because the start happened manually.

        cfg, self.config_file = tempfile.mkstemp()
        os.write(cfg, self.config)
        os.close(cfg)

        self.daemon = subprocess.Popen([self.DAEMON_PATH,
                                        '--config', self.config_file])

        time.sleep(1.0) # Allow some time for daemon to initialize...

    def stop(self):
        if self.daemon is not None:
            self.daemon.kill()
        if self.config_file is not None:
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
    'INVALID_DO_ID': 0,
    'INVALID_ZONE': 0,
    'RESERVED_MSG_TYPE': 0,

    # Success booleans
    'SUCCESS': 1,
    'FAILURE': 0,

    # Reserved Channels
    'INVALID_CHANNEL': 0,
    'CONTROL_CHANNEL': 1,
    'PARENT_PREFIX': 1 << 32,

    # Control message-type constants
    'CONTROL_ADD_CHANNEL':          9000,
    'CONTROL_REMOVE_CHANNEL':       9001,
    'CONTROL_ADD_RANGE':            9002,
    'CONTROL_REMOVE_RANGE':         9003,
    'CONTROL_ADD_POST_REMOVE':      9010,
    'CONTROL_CLEAR_POST_REMOVE':    9011,

    # State Server control message-type constants
    'STATESERVER_CREATE_OBJECT_WITH_REQUIRED':          2000,
    'STATESERVER_CREATE_OBJECT_WITH_REQUIRED_OTHER':    2001,
    'STATESERVER_DELETE_AI_OBJECTS':                    2009,
    # State Server object message-type constants
    'STATESERVER_OBJECT_GET_FIELD':         2010,
    'STATESERVER_OBJECT_GET_FIELD_RESP':    2011,
    'STATESERVER_OBJECT_GET_FIELDS':        2012,
    'STATESERVER_OBJECT_GET_FIELDS_RESP':   2013,
    'STATESERVER_OBJECT_GET_ALL':           2014,
    'STATESERVER_OBJECT_GET_ALL_RESP':      2015,
    'STATESERVER_OBJECT_SET_FIELD':         2020,
    'STATESERVER_OBJECT_SET_FIELDS':        2021,
    'STATESERVER_OBJECT_DELETE_FIELD_RAM':  2030,
    'STATESERVER_OBJECT_DELETE_FIELDS_RAM': 2031,
    'STATESERVER_OBJECT_DELETE_RAM':        2032,
    # State Server visibility message-type constants
    'STATESERVER_OBJECT_SET_LOCATION':                          2040,
    'STATESERVER_OBJECT_CHANGING_LOCATION':                     2041,
    'STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED':          2042,
    'STATESERVER_OBJECT_ENTER_LOCATION_WITH_REQUIRED_OTHER':    2043,
    'STATESERVER_OBJECT_GET_LOCATION':                          2044,
    'STATESERVER_OBJECT_GET_LOCATION_RESP':                     2045,
    'STATESERVER_OBJECT_SET_AI':                                2050,
    'STATESERVER_OBJECT_CHANGING_AI':                           2051,
    'STATESERVER_OBJECT_ENTER_AI_WITH_REQUIRED':                2052,
    'STATESERVER_OBJECT_ENTER_AI_WITH_REQUIRED_OTHER':          2053,
    'STATESERVER_OBJECT_GET_AI':                                2054,
    'STATESERVER_OBJECT_GET_AI_RESP':                           2055,
    'STATESERVER_OBJECT_SET_OWNER':                             2060,
    'STATESERVER_OBJECT_CHANGING_OWNER':                        2061,
    'STATESERVER_OBJECT_ENTER_OWNER_WITH_REQUIRED':             2062,
    'STATESERVER_OBJECT_ENTER_OWNER_WITH_REQUIRED_OTHER':       2063,
    'STATESERVER_OBJECT_GET_OWNER':                             2064,
    'STATESERVER_OBJECT_GET_OWNER_RESP':                        2065,
    # State Server parent methods message-type constants
    'STATESERVER_OBJECT_GET_ZONE_OBJECTS':      2100,
    'STATESERVER_OBJECT_GET_ZONES_OBJECTS':     2102,
    'STATESERVER_OBJECT_GET_CHILDREN':          2104,
    'STATESERVER_OBJECT_GET_ZONE_COUNT':        2110,
    'STATESERVER_OBJECT_GET_ZONE_COUNT_RESP':   2111,
    'STATESERVER_OBJECT_GET_ZONES_COUNT':       2112,
    'STATESERVER_OBJECT_GET_ZONES_COUNT_RESP':  2113,
    'STATESERVER_OBJECT_GET_CHILD_COUNT':       2114,
    'STATESERVER_OBJECT_GET_CHILD_COUNT_RESP':  2115,
    'STATESERVER_OBJECT_DELETE_ZONE':           2120,
    'STATESERVER_OBJECT_DELETE_ZONES':          2122,
    'STATESERVER_OBJECT_DELETE_CHILDREN':       2124,
    # DBSS object message-type constants
    'DBSS_OBJECT_ACTIVATE_DEFAULTS':        2200,
    'DBSS_OBJECT_ACTIVATE_DEFAULTS_OTHER':  2201,
    'DBSS_OBJECT_DELETE_FIELD_DISK':        2230,
    'DBSS_OBJECT_DELETE_FIELDS_DISK':       2231,
    'DBSS_OBJECT_DELETE_DISK':              2232,

    # Database Server
    'DBSERVER_CREATE_OBJECT':                       3000,
    'DBSERVER_CREATE_OBJECT_RESP':                  3001,
    'DBSERVER_OBJECT_GET_FIELD':                    3010,
    'DBSERVER_OBJECT_GET_FIELD_RESP':               3011,
    'DBSERVER_OBJECT_GET_FIELDS':                   3012,
    'DBSERVER_OBJECT_GET_FIELDS_RESP':              3013,
    'DBSERVER_OBJECT_GET_ALL':                      3014,
    'DBSERVER_OBJECT_GET_ALL_RESP':                 3015,
    'DBSERVER_OBJECT_SET_FIELD':                    3020,
    'DBSERVER_OBJECT_SET_FIELDS':                   3021,
    'DBSERVER_OBJECT_SET_FIELD_IF_EQUALS':          3022,
    'DBSERVER_OBJECT_SET_FIELD_IF_EQUALS_RESP':     3023,
    'DBSERVER_OBJECT_SET_FIELDS_IF_EQUALS':         3024,
    'DBSERVER_OBJECT_SET_FIELDS_IF_EQUALS_RESP':    3025,
    'DBSERVER_OBJECT_SET_FIELD_IF_EMPTY':           3026,
    'DBSERVER_OBJECT_SET_FIELD_IF_EMPTY_RESP':      3027,
    'DBSERVER_OBJECT_DELETE_FIELD':                 3030,
    'DBSERVER_OBJECT_DELETE_FIELDS':                3031,
    'DBSERVER_OBJECT_DELETE':                       3032,

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
            print "Channels don't match"
            return False

        self.seek(8*ord(self._data[0])+1)
        if sender != self.read_uint64():
            print "Sender doesn't match"
            return False

        if msgtype != self.read_uint16():
            print "MsgType doesn't match"
            return False

        if remaining != -1 and remaining != len(self._data) - self._offset:
            print "Remaining doesn't match"
            return False

        return True

    def read_uint8(self):
        self._offset += 1
        if self._offset > len(self._data):
            raise EOFError('End of Datagram')

        return struct.unpack("<B", self._data[self._offset-1:self._offset])[0]

    def read_uint16(self):
        self._offset += 2
        if self._offset > len(self._data):
            raise EOFError('End of Datagram')

        return struct.unpack("<H", self._data[self._offset-2:self._offset])[0]

    def read_uint32(self):
        self._offset += 4
        if self._offset > len(self._data):
            raise EOFError('End of Datagram')

        return struct.unpack("<I", self._data[self._offset-4:self._offset])[0]

    def read_uint64(self):
        self._offset += 8
        if self._offset > len(self._data):
            raise EOFError('End of Datagram')

        return struct.unpack("<Q", self._data[self._offset-8:self._offset])[0]

    def read_string(self):
        length = self.read_uint16()
        self._offset += length
        if self._offset > len(self._data):
            raise EOFError('End of Datagram')

        return struct.unpack("<%ds" % length, self._data[self._offset-length:self._offset])[0]

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

def ChannelConnection(MDConnection):
    def __init__(self, connAddr, connPort, MDChannel):
        c = socket(AF_INET, SOCK_STREAM)
        c.connect(('127.0.0.1', 57123))
        MDConnection.__init__(self, c)

        self.channels = [MDChannel]
        self.send(Datagram.create_add_channel(MDChannel))

    def add_channel(channel):
        if channel not in self.channels:
            self.channels.append(channel)
            self.send(Datagram.create_add_channel(channel))

    def remove_channel(channel):
        if channel in self.channels:
            self.send(Datagram.create_remove_channel(channel))
            self.channels.remove(channel)

    def close(self):
        for chan in self.channels:
            self.send(Datagram.create_remove_channel(chan))
        self.c.close()
