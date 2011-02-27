#include "hostinfo.hpp"

#ifdef WINDOWS
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif

using namespace Msync;

HostInfo::HostInfo(unsigned int id) :
    id(htonl(id))
{
}

bool HostInfo::operator==(const HostInfo& info) const
{
    return id == info.id;
}

bool HostInfo::operator<(const HostInfo& info) const
{
    return id < info.id;
}

unsigned int HostInfo::get_id() const
{
    return ntohl(id);
}
