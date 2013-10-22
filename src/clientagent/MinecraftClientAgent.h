//
//  MinecraftClientAgent.h
//  
//
//  Created by Alex Mault on 10/21/13.
//
//

#pragma once

#include "ClientMessages.h"
#include "ClientFactory.h"
#include "ClientAgent.h"

#include <boost/asio.hpp>
#include <boost/bind.hpp>

#include "core/global.h"
#include "util/Role.h"
#include "core/RoleFactory.h"
#include "util/Datagram.h"

#include <queue>
#include <set>
#include <list>
#include <algorithm>

#include <iostream>
#include "ClientAgent.h"

class MinecraftClient : Client  {
  

  
public:
    MinecraftClient(boost::asio::ip::tcp::socket *socket, LogCategory *log, RoleConfig roleconfig,
           ChannelTracker *ct);
    
};

