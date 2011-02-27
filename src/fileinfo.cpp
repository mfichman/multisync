
#include "fileinfo.hpp"
#include "blockserver.hpp"
#include "md5.hpp"
#include <cstring>
#include <fstream>

using namespace Msync;

FileInfo::FileInfo(const std::string& path)
{
    // Read the file's size
    std::ifstream input(path.c_str());
    input.seekg(0, std::ios::end);
    num_blocks = input.tellg() / BLOCKSIZE;
    if (input.tellg() % BLOCKSIZE) {
        num_blocks++;
    }
    num_blocks = htonl(num_blocks);
    input.seekg(0, std::ios::beg);
    
    // Get the MD5 digest for the whole input
    MD5 context(input);
    char* digest = context.hex_digest();
    std::copy(digest, digest + sizeof(this->digest), this->digest);
    delete digest;
}

bool FileInfo::operator==(const FileInfo& other) const
{
    return memcmp(digest, other.digest, sizeof(this->digest)) && (num_blocks == other.num_blocks);
}

bool FileInfo::operator<(const FileInfo& other) const
{   
    int val = memcmp(digest, other.digest, sizeof(this->digest));
    if (val != 0) {
        return val < 0;   
    } else {
        return num_blocks < other.num_blocks;   
    }
}

unsigned int FileInfo::get_block_count() const
{
    return ntohl(num_blocks);
}

const char* FileInfo::get_digest() const
{
    return digest;
}
