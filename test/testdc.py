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
setADb3 = 10

setX = 11
setY = 12
setUnrelated = 13
setZ = 14
setXyz = 15
setOne = 16
setTwo = 17
setThree = 18
set123 = 19
setRDbD5 = 20
setFoo = 21

request = 22
response = 23

foo = 24
bar = 25

setName = 26
setColor = 27
requestKill = 28
sendMessage = 29

setRequired1DefaultValue = 78

# If you edit test.dc *AT ALL*, you will have to recalculate this.
# If you don't know how, ask CFS.
DC_HASH = 0x57b2b45

setRDbD5DefaultValue = 20
