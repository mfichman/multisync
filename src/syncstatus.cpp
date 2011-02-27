#include "syncstatus.hpp"

using namespace Msync;

SyncStatus::SyncStatus(const FileInfo& info, const Address& server_address) :
    temp(std::string(tmpnam(NULL)) + info.get_digest() + ".msync"),
    block_array(info.get_block_count(), false),
    remaining_blocks(info.get_block_count()),
    output(new std::ofstream(temp.c_str(), std::ios::binary)),
	server_address(server_address)
{
}
    
void SyncStatus::write_block(unsigned long block, const Message& message)
{
    if (!block_array[block]) {
        // Seek to the location where the block should go, and write
        // the block to the output stream
        output->seekp(BLOCKSIZE * block);
        (*output) << message;
                
        // Mark the block as written.
        block_array[block] = true;
        remaining_blocks--;
    }
}
    
void SyncStatus::set_path(const std::string& path)
{
    this->path = path;
}
    
void SyncStatus::set_goodbye_received()
{
	this->goodbye_received = true;
}

bool SyncStatus::transfer_complete()
{
    if (remaining_blocks == 0 && !path.empty()) {
		if (output) {
			output->close();
			std::cout << "closing, moving " << temp << " to " << path << std::endl;
        	rename(temp.c_str(), path.c_str());
		} else {
			output.reset();
		}
        return true;
    } else {
        return false;
    }
}

const Address& SyncStatus::get_server_address() {
	return this->server_address;
}

bool SyncStatus::sync_complete()
{
	return goodbye_received && transfer_complete();
}
