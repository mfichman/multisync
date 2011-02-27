#ifndef BLOCKSERVER_HPP
#define BLOCKSERVER_HPP

#ifdef WINDOWS
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif

#include <string>
#include <set>
#include <map>
#include <vector>
#include <list>
#include <iostream>
#include <fstream>
#include "blocksocket.hpp"
#include "hostinfo.hpp"
#include "logger.hpp"
#include "fileinfo.hpp"

#define BLOCKSIZE 1024

namespace Msync {

class BlockServer {

public:

    /**
     * Creates a new block server with the given attributes.
     * @param source the path to the file to serve
     * @param path the destination path
     * @param group the multicast group address
     * @param port the port number
     * @param logger the logger to use
     */
    BlockServer(const std::string& source, const std::string& path,
        const std::string& group = "228.5.6.7", unsigned short port = 9000,
		Logger& logger = Logger::Default);	

    /**
     * Opens the socket and begins serving the file.
     * @throw string if the operation fails
     */
    void start();

private:

	typedef void (BlockServer::*message_handler)(const Message&, const Address&);
    
    /**
     * Loops while processing messages and the send queue for the given amount
     * of time.
     * @param timeout the time to loop in milliseconds
     * @throw string on I/O error
     */
    void select(long timeout);
    
    /**
     * Processes one incoming message from the underlying socket.
     */
    void process_message();

	void handle_chello(const Message& message, const Address& address);
	void handle_getinfo(const Message& message, const Address& address);
	void handle_getblock(const Message& message, const Address& address);
	void handle_cgoodbye(const Message& message, const Address& address);

    BlockSocket socket;
    unsigned int id;
    std::string path;
    std::ifstream input;
    std::set<HostInfo> host_info;
    std::list<Message> message_queue;
    FileInfo file_info;
    Logger logger;
	std::map<unsigned int, message_handler> handlers;
};

}

#endif




//#void (:) hi computer....< helpmatt so he can go on vacation..c://{} !2@kthanx>>
