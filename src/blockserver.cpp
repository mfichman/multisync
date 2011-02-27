#include "blockserver.hpp"
#include "blockinfo.hpp"

#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif

#ifdef WINDOWS
#include <winsock2.h>
#else
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#endif

#include <cstring>
#include <cstdlib>
#include <string>

using namespace Msync;

BlockServer::BlockServer(const std::string& source, const std::string& path, 
        const std::string& group, unsigned short port, Logger& logger) : 
    socket(group, 0),
    id(rand()),
    path(path),
    input(source.c_str(), std::ios::binary),
    file_info(source),
    logger(logger)
{   
	socket << Address(group, port);
    logger << Logger::INFO << "File " << source << " has " << file_info.get_block_count() << " blocks\n";

	handlers[MESSAGE_TYPE_CHELLO] = &BlockServer::handle_chello;
	handlers[MESSAGE_TYPE_GETINFO] = &BlockServer::handle_getinfo;
	handlers[MESSAGE_TYPE_GETBLOCK] = &BlockServer::handle_getblock;
	handlers[MESSAGE_TYPE_CGOODBYE] = &BlockServer::handle_cgoodbye;
}

void BlockServer::start()
{
    socket.open();
    
    // Send the file information
    message_queue.push_back(Message(id, MESSAGE_TYPE_INFO, file_info, path));
    logger << Logger::INFO << "Sending initial file information\n";
    
    // Begin serving the blocks.  It doesn't matter if the clients can't
    // get the blocks; missing blocks will be resent later.
    for (BlockInfo i(file_info); i < file_info.get_block_count(); i++) {
    
        // Make sure the message queue doesn't grow beyond size 5...we don't 
        // want to use too much memory
        if (message_queue.size() < 5) {
            message_queue.push_back(Message(id, MESSAGE_TYPE_BLOCK, i, BLOCKSIZE));
            Message& message = message_queue.back();
            input.seekg(BLOCKSIZE * i);
            input >> message;
            
            logger << Logger::INFO << "Enqueueing block #" << i << " (" << message.get_length() << " bytes)\n";
        }
        select(-1);
    }
    
    // Now process remaining requests until we're finished
    while (!host_info.empty() || message_queue.size() > 0) {
        select(-1);
    }
}

void BlockServer::select(long timeout)
{
    // Read/write any oustanding messages
    BlockSocket::Status status = socket.select(timeout, !message_queue.empty());
    if (status == BlockSocket::READ || status == BlockSocket::BOTH) {
        process_message();
    } 
    if (status == BlockSocket::WRITE || status == BlockSocket::BOTH) {
        socket << message_queue.front();
        message_queue.pop_front();
    }
}

void BlockServer::process_message()
{
    Message message(4096);
	Address address;
    socket >> message >> address;
    

	typedef std::map<unsigned int, message_handler> handler_map;
	handler_map::iterator i = handlers.find(message.get_type());
	if (i != handlers.end()) {	
		message_handler handler = i->second;
		(this->*handler)(message, address);
	} else {
		logger << Logger::FINE << "Unknown message type!\n";
	}
}


void BlockServer::handle_chello(const Message& message, const Address& address)
{
    // The client wishes to be added to the client list
	const HostInfo& info = message.get_metadata<HostInfo>();
    host_info.insert(info);
    logger << Logger::FINE << "Found host " << info.get_id() << "\n";
}

void BlockServer::handle_getinfo(const Message& message, const Address& address)
{
    // The client has requested information about the file served by 
    // this server instance.
	// Add this host to the set of remaining hosts
    host_info.insert(message.get_metadata<HostInfo>());
    message_queue.push_back(Message(id, MESSAGE_TYPE_INFO, file_info, path)); 
    logger << Logger::FINE << "Request from host for file information\n";
}

void BlockServer::handle_getblock(const Message& message, const Address& address)

{
    // The client has requested a specific block from the served file.
	host_info.insert(message.get_metadata<HostInfo>());
    const BlockInfo& i = message.get_metadata<BlockInfo>();
    if (i.get_file_info() == file_info) {
    	message_queue.push_back(Message(id, MESSAGE_TYPE_BLOCK, i, BLOCKSIZE));
        input.seekg(BLOCKSIZE * i);
        input >> message_queue.back();
    }
    logger << Logger::FINE << "Request from host for block " << i << "\n";
}

void BlockServer::handle_cgoodbye(const Message& message, const Address& address)
{
    // The client has finished receiving all blocks, and will shut down.
	const HostInfo& i = message.get_metadata<HostInfo>();
    logger << Logger::INFO << "Host " << i.get_id() << " is shutting down\n";
    host_info.erase(i);            
}
