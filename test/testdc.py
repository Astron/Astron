# KEEP THIS FILE IN SYNC WITH test.dc! This contains constants used in
# unit tests.

import os.path

test_dc = os.path.abspath(os.path.join(os.path.dirname(__file__), 'test.dc'))

DistributedTestObject1 = 0
DistributedTestObject2 = 1
DistributedTestObject3 = 2
DistributedTestObject4 = 3
DistributedTestObject5 = 4
UberDog1 = 5
UberDog2 = 6
DistributedClientTestObject = 7

setRequired1 = 0
setB1 = 1
setBA1 = 2
setBR1 = 3
setBRA1 = 4
setBRO1 = 5

setB2 = 6
setBRam2 = 7

setDb3 = 8
setRDB3 = 9
setX = 10
setY = 11
setUnrelated = 12
setZ = 13
setXyz = 14
setOne = 15
setTwo = 16
setThree = 17
set123 = 18
setRDbD5 = 19
setFoo = 20

request = 21
response = 22

foo = 23
bar = 24

setName = 25
setColor = 26
requestKill = 27
sendMessage = 28

setRequired1DefaultValue = 78

# If you edit test.dc *AT ALL*, you will have to recalculate this.
# If you don't know how, ask CFS.
DC_HASH = 0x53c5b3b

setRDbD5DefaultValue = 20
