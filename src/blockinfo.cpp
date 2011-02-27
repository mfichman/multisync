
#include "blockinfo.hpp"

#ifdef WINDOWS
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif

using namespace Msync;

BlockInfo::BlockInfo(const FileInfo& info, unsigned int block) :
    file_info(info),
    block_num(htonl(block))
{
}

bool BlockInfo::operator==(const BlockInfo& other) const
{
    return (file_info == other.file_info) && (block_num == other.block_num);
}

void BlockInfo::operator++(int)
{
    block_num = htonl(ntohl(block_num) + 1);
}

BlockInfo::operator unsigned int() const
{
    return ntohl(block_num);
}

unsigned int BlockInfo::get_block() const
{
    return ntohl(block_num);
}

const FileInfo& BlockInfo::get_file_info() const
{
    return file_info;
}
