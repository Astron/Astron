import subprocess
import tempfile
import struct
import socket
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
            length = struct.unpack('<H', self.s.recv(2))[0]
        except socket.error:
            return None

        result = ''
        while len(result) < length:
            result += self.s.recv(length - len(result))

        return result

    def close(self):
        self.s.close()

    def flush(self):
        while self._read(): pass

    def expect(self, datagram):
        return self._read() == datagram.get_data()

    def expect_none(self):
        return self._read() == None
