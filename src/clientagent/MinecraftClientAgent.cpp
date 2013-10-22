//
//  MinecraftClientAgent.cpp
//  
//
//  Created by Alex Mault on 10/21/13.
//
//


#include "MinecraftClientAgent.h"


MinecraftClient::MinecraftClient(boost::asio::ip::tcp::socket *socket,
                                 LogCategory *log,
                                 RoleConfig roleconfig,
                                 ChannelTracker *ct) : Client(socket, log, roleconfig, ct)
{
    
}





static ClientType<Client> client_type(1);
