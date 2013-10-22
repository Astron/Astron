//
//  MinecraftClientMessages.h
//  
//
//  Created by Alex Mault on 10/21/13.
//
//


#pragma once

#define KEEP_ALIVE                  0x0
#define LOGIN_REQUEST               0x1
#define HANDSHAKE                   0x2
#define CHAT_MESSAGE                0x3
#define TIME_UPDATE                 0x4
#define ENTITY_EQUIPMENT            0x5
#define SPAWN_POSITION              0x6
#define USE_ENTITY                  0x7
#define UPDATE_HEALTH               0x8
#define RESPAWN                     0x9
#define PLAYER                      0xA
#define PLAYER_POSITION             0xB
#define PLAYER_LOOK                 0xC
#define PLAYER_POSITION_AND_LOOK    0xD
#define PLAYER_DIGGING              0xE
#define PLAYER_BLOCK_PLACEMENT      0xF
#define HELD_ITEM_CHANGE            0x10
#define USE_BED                     0x11
#define ANIMATION                   0x12
#define ENTITY_ACTION               0x13
#define SPAWN_NAMED_ENTITY          0x14
//0x15 not defined
#define COLLECT_ITEM                0x16
#define SPAWN_OBJECT_VEHICLE        0x17
#define SPAWN_MOB                   0x18
#define SPAWN_PAINTING              0x19
#define SPAWN_EXP_ORB               0x1A
#define STEER_VEHICLE               0x1B
#define ENTITY_VELOCITY             0x1C
#define DESTROY_ENTITY              0x1D
#define ENTITY                      0x1E
#define ENTITY_RELATIVE_MOVE        0x1F
#define ENTITY_LOOK                 0x20
#define ENTITY_LOOK_RELATIVE_MOVE   0x21
#define ENTITY_TELEPORT             0x22
#define ENTITY_HEAD_LOOK            0x23
//0x24
//0x25
#define ENTITY_STATUS               0x26
#define ATTACH_ENTITY               0x27
#define ENTITY_METADATA             0x28
#define ENTITY_EFFECT               0x29
#define ENTITY_REMOVE_EFFECT        0x2A
#define SET_EXP                     0x2B
#define ENTITY_PROPERTIES           0x2C
//LOTS OF SPACE...
#define CHUNK_DATA                  0x33
#define MULTI_BLOCK_CHANGE          0x34
#define BLOCK_CHANGE                0x35
#define BLOCK_ACTION                0x36
#define BLOCK_BREAK_ANIMATION       0x37
#define MAP_CHUNK_BULK              0x38
//MORE TO BE ADDED!

#define ENCRYPTION_KEY_RESPONSE     0xFC
#define ENCRYPTION_KEY_REQUEST      0xFD
#define SERVER_LIST_PING            0xFE
#define DISCONNECT_KICK_CLIENT      0xFF



