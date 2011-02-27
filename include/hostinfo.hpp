#ifndef HOSTINFO_HPP
#define HOSTINFO_HPP

#include "hostinfo.hpp"

namespace Msync {

class HostInfo {
public:

    /**
     * Creates a new host info to store statistics about a host.
     * @param guid the host id (usually the IP address)
     */
    HostInfo(unsigned int id);
    
    /**
     * Determines whether or not this host is equal to another.
     * @return true if both ids are equal
     */
    bool operator==(const HostInfo& other) const;
    
    /**
     * Compares this host to another host.
     * @return true if this host is less than the other
     */
    bool operator<(const HostInfo& other) const;
    
    /**
     * Returns the block number
     * @return the block number
     */
    unsigned int get_id() const;

private:
    unsigned int id;
};

}

#endif
