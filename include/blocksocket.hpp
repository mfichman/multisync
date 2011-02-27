#ifndef BLOCKSOCKET_HPP
#define BLOCKSOCKET_HPP

#include <string>
#include <set>

#ifdef WINDOWS
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif

#include <vector>
#include "message.hpp"
#include "logger.hpp"

namespace Msync {

struct Address {
	Address(const std::string& ip_address, unsigned short port) :
		ip_address(ip_address),
		port(port)
    {
	}
	
	Address() {}

	std::string ip_address;
	unsigned short port;
};


class BlockSocket {
public:

    enum Status { READ, WRITE, BOTH, NONE };

    /**
     * Creates a new block socket with the given attributes.
     * @param group the multicast group address
     * @param port the port number to listen to
     */
    BlockSocket(const std::string& group, unsigned short port, Logger& logger = Logger::Default);

    /**
     * Closes the underlying block socket if not already closed.
     */
    ~BlockSocket();
    
    /**
     * Binds the socket to the multicast group and port.
     * @throw string error if the operation fails
     */
    void open();
    
    /**
     * Waits for activity on the underlying socket.
     * @param the timeout to use
     * @param poll_write whether to poll for write events
     * @throw string error of the operation fails
     * @return the status of the select call
     */
    Status select(long timeout, bool poll_write = false);
    
    /**
     * Waits for activity on the underlying socket.  Uses the default timeout
     * value.
     * @throw string error of the operation fails
     * @return the status of the select call
     */
    Status select();
    
    /**
     * Reads from a block into a message.
     * @param message the message to read into
     * @throw string error if the operation fails
     */
    BlockSocket& operator>>(Message& message);
    
    /**
     * Writes a message to the socket.
     * @param message the message to write
     * @throw string error if the operation fails
     */
    BlockSocket& operator<<(const Message& message);
	
	/**
	 * Gets the last address of the socket.
	 * @param last the last received address
	 */
	BlockSocket& operator>>(Address& address);
	
	/**
	 * Sets the destination address of the socket.
	 * @param dest the destination
	 */
	BlockSocket& operator<<(const Address& address);

    /**
     * Closes the underlying socket.  The socket can be reponed with open().
     */
    void close();
    
    /**
     * Sets the timeout length, in milliseconds.
     * @param timeout the timeout length in milliseconds
     */
    void set_timeout(long timeout);
    
private:
    int sock;
    sockaddr_in group;
    sockaddr_in from;
	sockaddr_in to;
    long timeout;
    Logger& logger;
	unsigned short port;
};

}

#endif
