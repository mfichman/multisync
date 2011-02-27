#ifndef FILEINFO_HPP
#define FILEINFO_HPP

#include <string>

namespace Msync {

class FileInfo {
public:

    /**
     * Creates a new object to store statistics about a file.
     * @param path the path to the file to stat
     */
    FileInfo(const std::string& path);
    
    /**
     * Determines whether or not this file info is equal to another file's
     * info.
     * @return true if both the hash digest and file length of both files
     * are equal
     */
    bool operator==(const FileInfo& other) const;
    
    /**
     * Determines the ordering of this file info.
     * @return true if this file info is less than the other.
     */
    bool operator<(const FileInfo& other) const;
    
    /**
     * Returns the number of blocks in the file.
     * @return the number of blocks
     */
    unsigned int get_block_count() const;
    
    /**
     * Returns a pointer to the digest.
     * @return pointer to the array digest
     */
    const char* get_digest() const;
     

private:
    char digest[33];
    unsigned int num_blocks;
};

}

#endif
