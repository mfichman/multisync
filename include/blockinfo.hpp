#ifndef BLOCKINFO_HPP
#define BLOCKINFO_HPP

#include "fileinfo.hpp"
#include <string>

namespace Msync {

class BlockInfo {
public:

    /**
     * Creates a new object to store statistics about a file block.
     * @param info the file info
     * @param block the block number
     */
    BlockInfo(const FileInfo& info, unsigned int block = 0);
    
    /**
     * Determines whether or not this file info is equal to another file's
     * info.
     * @return true if both the hash digest and file length of both files
     * are equal
     */
    bool operator==(const BlockInfo& other) const;
    
    /**
     * Increments the block number of this BlockInfo object.
     */
    void operator++(int);
    
    /**
     * Casts the block info into an unsigned integer.
     * @return the int value of this block info
     */
    operator unsigned int() const;
    
    /**
     * Returns the block number
     * @return the block number
     */
    unsigned int get_block() const;  
    
    /**
     * Returns the file into object.
     * @return information about the file this block came from
     */
    const FileInfo& get_file_info() const;   

private:
    FileInfo file_info;
    unsigned int block_num;
};

}

#endif
