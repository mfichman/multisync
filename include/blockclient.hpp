#ifndef BLOCKCLIENT_HPP
#define BLOCKCLIENT_HPP


#define STATE_OPEN      0
#define STATE_WAITING   1
#define STATE_READING   2
#define STATE_CLOSED    3

#include "blocksocket.hpp"
#include "fileinfo.hpp"
#include "blockinfo.hpp"
#include "syncstatus.hpp"
#include <string>
#include <vector>
#include <list>
#include <map>
#include <fstream>
#include <memory>

#ifndef WINDOWS
#include <tr1/memory>
#endif

namespace Msync {

class BlockClient {
public:
    
    /**
     * Creates a new block client with the given attributes.
     * @param group the multicast group address
     * @param port the port number
     */
    BlockClient(const std::string& group = "228.5.6.7",  
		unsigned short port = 9000, Logger& logger = Logger::Default);

    /**
     * Binds the socket and prepares to listen for messages.
     * @throw string if the operation fails
     */
    void start();

private:

	typedef void (BlockClient::*message_handler)(const Message&, const Address&);
    
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
    
    /**
     * Saves file information.
     * @param info the file info
     * @param path the file path
     */
    void save_info(const FileInfo& info, const std::string& path);
    
    /**
     * Returns the synchronization status associated with the given file
     * identification information.
     * @param info file information.
     * @return the status of the given file.
     */
    SyncStatus& get_sync_status(const FileInfo& info, const Address& address);
    
    /**
     * Checks the sync status to see if the file transfer is complete.
     * @param status the status object
     */
    void check_sync_status(const SyncStatus& status);

	void handle_info(const Message& message, const Address& address);
	void handle_block(const Message& message, const Address& address);
    void handle_sgoodbye(const Message& message, const Address& address);

    HostInfo info;
    Logger logger;
    BlockSocket socket;
    std::map<FileInfo, SyncStatus> sync_set;
    std::list<Message> message_queue;
	std::map<unsigned int, message_handler> handlers;
    unsigned int id;
};

}

/**
 * 1. Open: Initialize the UDP socket
 * 2. Wait: Wait for first packet from the server
 * 3. Read attributes
 * 4. Read: Notify the server that we're ready and get data
 * 5. Close: Close the connection
 */

#endif
