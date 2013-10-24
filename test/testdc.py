# KEEP THIS FILE IN SYNC WITH test.dc! This contains constants used in
# unit tests.

import os.path

test_dc = os.path.abspath(os.path.join(os.path.dirname(__file__), 'test.dc'))

### Class and struct ids ###
DistributedTestObject1 = 0
DistributedTestObject2 = 1
DistributedTestObject3 = 2
DistributedTestObject4 = 3
DistributedTestObject5 = 4
UberDog1 = 5
UberDog2 = 6
DistributedClientTestObject = 7
Block = 8
DistributedChunk = 9

### Fields for DistributedTestObject1 ###
setRequired1 = 0
setB1 = 1
setBA1 = 2
setBR1 = 3
setBRA1 = 4
setBRO1 = 5

### Fields for DistributedTestObject2 ###
setB2 = 6
setBRam2 = 7

### Fields for DistributedTestObject3 ###
setDb3 = 8
setRDB3 = 9

### Fields for DistributedTestObject4 ###
setX = 10
setY = 11
setUnrelated = 12
setZ = 13
setXyz = 14
setOne = 15
setTwo = 16
setThree = 17
set123 = 18

### Fields for DistributedTestObject5 ###
setRDbD5 = 19
setFoo = 20

### Fields for UberDog1 ###
request = 21
response = 22

### Fields for UberDog2 ###
foo = 23
bar = 24

### Fields for DistributedClientTestObject ###
setName = 25
setColor = 26
requestKill = 27
sendMessage = 28

### Fields for Block ###
blockX = 29
blockY = 30
blockZ = 31

### Fields for DistributedChunk ###
blockList = 32
lastBlock = 33
newBlock = 34

### Default field values ###
setRequired1DefaultValue = 78
setRDbD5DefaultValue = 20

# If you edit test.dc *AT ALL*, you will have to recalculate this.
# If you don't know how, ask CFS.
DC_HASH = 0x75ecf80
