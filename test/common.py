import subprocess
import tempfile
import struct
import socket
import time
import os

__all__ = ['Daemon', 'Datagram', 'DatagramIterator', 'MDConnection',
           'ClientConnection']

class Daemon(object):
    DAEMON_PATH = './openotpd'

    def __init__(self, config):
        self.config = config

        self.daemon = None
        self.config_file = None

    def start(self):
        if 'MANUAL_LAUNCH_CONFIG' in os.environ:
            # User wants to manually launch their OpenOTP daemon, so we'll write
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
    'INVALID_CHANNEL': 0,
    'INVALID_DO_ID': 0,
    'INVALID_MSG_TYPE': 0,

    # Defined return codes
    'FOUND': 1,
    'NOT_FOUND': 0,

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
    'STATESERVER_OBJECT_QUERY_FIELD': 2024,
    'STATESERVER_OBJECT_QUERY_FIELD_RESP': 2062,
    'STATESERVER_OBJECT_QUERY_FIELDS': 2080,
    'STATESERVER_OBJECT_QUERY_FIELDS_RESP': 2081,
    'STATESERVER_OBJECT_QUERY_ALL_RESP': 2030,
    'STATESERVER_OBJECT_QUERY_ZONE_ALL': 2021,
    'STATESERVER_OBJECT_QUERY_ZONE_ALL_DONE': 2046,
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

    # Client Agent
    'CLIENTAGENT_OPEN_CHANNEL': 3104,
    'CLIENTAGENT_CLOSE_CHANNEL': 3105,
    'CLIENTAGENT_ADD_INTEREST': 3106,
    'CLIENTAGENT_REMOVE_INTEREST': 3107,
    'CLIENTAGENT_ADD_POST_REMOVE': 3108,
    'CLIENTAGENT_CLEAR_POST_REMOVE': 3109,
    'CLIENTAGENT_DISCONNECT': 3101,
    'CLIENTAGENT_DROP': 3102,
    'CLIENTAGENT_SEND_DATAGRAM': 3100,
    'CLIENTAGENT_SET_SENDER_ID': 3103,
    'CLIENTAGENT_SET_STATE': 3110,
    'CLIENTAGENT_ADD_SESSION_OBJECT': 3112,
    'CLIENTAGENT_REMOVE_SESSION_OBJECT': 3113,
    'CLIENTAGENT_DECLARE_OBJECT': 3114,
    'CLIENTAGENT_UNDECLARE_OBJECT': 3115,
    'CLIENTAGENT_SET_FIELDS_SENDABLE': 3111,

    # Client
    'CLIENT_HELLO': 1,
    'CLIENT_HELLO_RESP': 2,
    'CLIENT_GO_GET_LOST': 4,
    'CLIENT_OBJECT_UPDATE_FIELD': 24,
    'CLIENT_OBJECT_DISABLE': 25,
    'CLIENT_CREATE_OBJECT_REQUIRED': 34,
    'CLIENT_CREATE_OBJECT_REQUIRED_OTHER': 35,
    'CLIENT_CREATE_OBJECT_REQUIRED_OTHER_OWNER': 36,
    'CLIENT_HEARTBEAT': 52,
    'CLIENT_ADD_INTEREST': 97,
    'CLIENT_REMOVE_INTEREST': 99,
    'CLIENT_DONE_INTEREST_RESP': 48,
    'CLIENT_OBJECT_LOCATION': 102,
    # Client DC reasons
    'CLIENT_DISCONNECT_OVERSIZED_DATAGRAM': 106,
    'CLIENT_DISCONNECT_NO_HELLO': 107,
    'CLIENT_DISCONNECT_INVALID_MSGTYPE': 108,
    'CLIENT_DISCONNECT_TRUNCATED_DATAGRAM': 109,
    'CLIENT_DISCONNECT_ANONYMOUS_VIOLATION': 113,
    'CLIENT_DISCONNECT_MISSING_OBJECT': 117,
    'CLIENT_DISCONNECT_FORBIDDEN_FIELD': 118,
    'CLIENT_DISCONNECT_FORBIDDEN_RELOCATE': 119,
    'CLIENT_DISCONNECT_BAD_VERSION': 124,
    'CLIENT_DISCONNECT_BAD_DCHASH': 125,
    'CLIENT_DISCONNECT_SESSION_OBJECT_DELETED': 153,
    'CLIENT_DISCONNECT_NO_HEARTBEAT': 345,
    'CLIENT_DISCONNECT_NETWORK_WRITE_ERROR': 347,
    'CLIENT_DISCONNECT_NETWORK_READ_ERROR': 348,
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

    def equals(self, other):
        return self.get_data() == other.get_data()

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
            if dg is None:
                print "No Datagram Recv'd"
                return False # Augh, we didn't see all the dgs yet!
            dg = Datagram(dg)

            for datagram in datagrams:
                if datagram.is_subset_of(dg):
                    datagrams.remove(datagram)
                    break
            else:
                if only:
                    print "Wrong Datagram Recv'd"
                    f = open("test.x", "wb")
                    f.write(dg.get_data())
                    f.close()
                    f = open("test.y", "wb")
                    f.write(datagram.get_data())
                    f.close()
                    return False

        return True


    def expect_none(self):
        return self._read() == None

class ClientConnection(MDConnection):
    def expect_multi(self, datagrams, only=False):
        datagrams = list(datagrams) # We're going to be doing datagrams.remove()

        while datagrams:
            dg = self._read()
            if dg is None:
                print "No Datagram Recv'd"
                return False # Augh, we didn't see all the dgs yet!
            dg = Datagram(dg)

            for datagram in datagrams:
                if datagram.equals(dg):
                    datagrams.remove(datagram)
                    break
            else:
                if only:
                    print "Wrong Datagram Recv'd"
                    return False

        return True
