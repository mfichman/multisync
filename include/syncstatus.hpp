#ifndef SYNCSTATUS_HPP
#define SYNCSTATUS_HPP


#include "fileinfo.hpp"
#include "blockserver.hpp"
#include "message.hpp"
#include <string>
#include <vector>
#include <fstream>

#ifdef WINDOWS
#include <memory>
#else
#include <tr1/memory>
#endif

namespace Msync {

class SyncStatus { 
public:
    /**
     * Creates a new sync status object from the given file information.
     * @param info the file information.
	 * @param server_address the server address
     */
    SyncStatus(const FileInfo& info, const Address& server_address);
    
    /**
     * Marks the given block as complete, and writes the block if it hasn't
     * already been written.
     * @param info the block info
     * @param message the message containing the block
     */
    void write_block(unsigned long block, const Message& message);
    
    /**
     * Sets the path to write the block to.
     * @param path the path
     */
    void set_path(const std::string& path);
	
	/**
	 * Gets the server address associated with this status.
	 * @return the address
	 */
	const Address& get_server_address();
    
    /**
     * Returns true if the sync is complete.
     * @return boolean true if the sync is complete
     */
    bool transfer_complete();
	
	/**
     * Returns true if the client and server have both acknowledged that the
	 * sync has completed.
	 * @return true if the sync is complete
	 */
	bool sync_complete();
    
    /**
     * This method is called when a goodbye is received from the server.
     */
    void set_goodbye_received();

private:
    std::string temp;
    std::string path;
    std::vector<bool> block_array;
    unsigned long remaining_blocks;
    std::tr1::shared_ptr<std::ofstream> output;
    bool goodbye_received;
	Address server_address;
};

}


#endif
