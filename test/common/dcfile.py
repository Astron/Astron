# KEEP THIS FILE IN SYNC WITH test.dc! This contains constants used in
# unit tests.

import os.path

test_dc = os.path.abspath(os.path.join(os.path.dirname(__file__), '../files/test.dc'))

### Class and struct ids ###
CLASSES = [
    'DistributedTestObject1',
    'DistributedTestObject2',
    'DistributedTestObject3',
    'DistributedTestObject4',
    'DistributedTestObject5',
    'UberDog1',
    'UberDog2',
    'DistributedClientTestObject',
    'Block',
    'DistributedChunk',
    'DistributedDBTypeTestObject',
]
for i,n in enumerate(CLASSES):
    locals()[n] = i

FIELDS = [
    ### Fields for DistributedTestObject1 ###
    'setRequired1',
    'setB1',
    'setBA1',
    'setBR1',
    'setBRA1',
    'setBRO1',

    ### Fields for DistributedTestObject2 ###
    'setB2',
    'setBRam2',

    ### Fields for DistributedTestObject3 ###
    'setDb3',
    'setRDB3',
    'setADb3',

    ### Fields for DistributedTestObject4 ###
    'setX',
    'setY',
    'setUnrelated',
    'setZ',
    'setXyz',
    'setOne',
    'setTwo',
    'setThree',
    'set123',

    ### Fields for DistributedTestObject5 ###
    'setRDbD5',
    'setFoo',

    ### Fields for UberDog1 ###
    'request',
    'response',

    ### Fields for UberDog2 ###
    'foo',
    'bar',

    ### Fields for DistributedClientTestObject ###
    'setName',
    'setColor',
    'requestKill',
    'sendMessage',

    ### Fields for Block ###
    'blockX',
    'blockY',
    'blockZ',

    ### Fields for DistributedChunk ###
    'blockList',
    'lastBlock',
    'newBlock',

    ### Fields for DistributedDBTypeTestObject ###
    'db_uint8',
    'db_uint16',
    'db_uint32',
    'db_uint64',
    'db_int8',
    'db_int16',
    'db_int32',
    'db_int64',
    'db_char',
    'db_float64',
    'db_string',
    'db_fixstr',
    'db_blob',
    'db_fixblob',
    'db_complex',
]
for i,n in enumerate(FIELDS):
    locals()[n] = i

### Default field values ###
setRequired1DefaultValue = 78
setRDbD5DefaultValue = 20

# If you edit test.dc *AT ALL*, you will have to recalculate this.
# If you don't know how, ask CFS.
DC_HASH = 0x9a71149
