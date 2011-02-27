#include "blocksocket.hpp"
#include "logger.hpp"

#ifdef WINDOWS
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <arpa/inet.h>
#endif

#ifndef INVALID_SOCKET
#define INVALID_SOCKET -1
#endif

#ifdef WINDOWS
const char* errmsg()
{
    static char buffer[512];
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, WSAGetLastError(), NULL, buffer, 512, NULL);
    return buffer;
}    
    
#else
#define errmsg() strerror(errno)
#endif

#include <cerrno>
#include <cstring>
#include <string>

using namespace Msync;

BlockSocket::BlockSocket(const std::string& group, unsigned short port, Logger& logger) : 
    sock(INVALID_SOCKET),
    timeout(5000),
    logger(logger)
{
    this->group.sin_family = AF_INET;
    this->group.sin_addr.s_addr = inet_addr(group.c_str());
    this->group.sin_port = htons(port);
	this->to.sin_family = AF_INET; 
	this->to.sin_addr.s_addr = inet_addr(group.c_str());
	this->to.sin_port = htons(port);
    
    
#ifdef WINDOWS
    WSAData data;
    if (WSAStartup(MAKEWORD(1, 1), &data) != 0) {
        throw std::string("Could not start Winsock");
    }
#endif
    
}

BlockSocket::~BlockSocket()
{
    if (sock != INVALID_SOCKET) {
        logger << Logger::INFO << "Disconnecting\n";
        close();
    }
#ifdef WINDOWS
    WSACleanup();
#endif
}

void BlockSocket::open()
{
    // Create a UDP socket
    logger << Logger::FINE << "Creating UDP socket\n";
    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock == INVALID_SOCKET) {
        logger << Logger::ERR << "Could not create socket\n";
        throw std::string(errmsg());
    }

	
	/*
    // Set reuse option so that multiple sockets can bind to the same
    // port on the same machine

    
    int yes = 1;
#ifdef SO_REUSEPORT
    logger << Logger::FINE << "Setting SO_REUSEPORT\n";
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, (char*)&yes, sizeof(yes)) < 0) {
        throw std::string(errmsg());
    }
#endif
    logger << Logger::FINE << "Setting SO_REUSEADDR\n";
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(yes)) < 0) {
        throw std::string(errmsg());
    }
	*/
    
    // Request membership in the multicast group
    logger << Logger::INFO << "Joining multicast group " << inet_ntoa(group.sin_addr) << "\n";
    ip_mreq mreq;
    mreq.imr_multiaddr.s_addr = group.sin_addr.s_addr;
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char*)&mreq, sizeof(ip_mreq)) < 0) {
        throw std::string(errmsg());
    }

    // Bind to the given port
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = group.sin_port;
    if (bind(sock, (sockaddr *)&address, sizeof(sockaddr)) < 0) {
        throw std::string(errmsg());
    }
	
	// Get the socket name
	socklen_t length = sizeof(sockaddr);
	if (getsockname(sock, (sockaddr *)&address, &length) < 0) {
		throw std::string(strerror(errno));
	}
    logger << Logger::INFO << "Bound to port " << ntohs(address.sin_port) << "\n";
}

BlockSocket::Status BlockSocket::select(long timeout, bool poll_read)
{
    fd_set readfds;
    fd_set writefds;
    timeval time;
    time.tv_sec = timeout / 1000;
    time.tv_usec = (timeout % 1000) * 1000;
    
    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);
    FD_ZERO(&writefds);
    FD_SET(sock, &writefds);
    

    int ret = ::select(sock + 1, &readfds, poll_read ? &writefds : NULL, NULL, timeout < 0 ? NULL : &time); 
    // If the return value is not positive, we don't have a message available
    if (ret < 0) {
        throw std::string(strerror(errno));
    } else if (ret > 0) {
        if (FD_ISSET(sock, &readfds) && FD_ISSET(sock, &writefds) && poll_read) {
            return BOTH;
        } else if (FD_ISSET(sock, &readfds)) {
            return READ;
        } else if (FD_ISSET(sock, &writefds)) {
            return WRITE;
        }
    }
    return NONE;
}

BlockSocket::Status BlockSocket::select()
{
    return select(this->timeout);
}

BlockSocket& BlockSocket::operator>>(Message& message)
{
    socklen_t fromlen = sizeof(sockaddr);
    
    int bytes = recvfrom(sock, &message.buffer.front(), message.buffer.size(), 0, (sockaddr*)&from, &fromlen);
    if (bytes < 0) {
        throw std::string(strerror(errno));
    } else if (bytes == 0) {
        throw std::string("Received zero bytes on UDP socket");
    }
    logger << Logger::FINE << "Received from " << inet_ntoa(from.sin_addr) << ":" << ntohs(from.sin_port) << "\n";
    
    // Trim the buffer down to the size of the packet
    message.buffer.resize(bytes);   
     
    // Return false if the size of the header plus the size reported in the
    // header isn't equal to the length of the whole packet
    Header* header = (Header*)&message.buffer.front();
    if (header->GetPacketLength() != (unsigned int)bytes) {
        logger << Logger::ERR << "Expected " << header->GetPacketLength() << " bytes, but received " << bytes << "\n";
        logger << Logger::ERR << "Data length: " << ntohl(header->length) << "\n";
        logger << Logger::ERR << "Header length: " << ntohs(header->offset) << "\n";
        throw std::string("Invalid packet length");
    }
	return *this;
}

BlockSocket& BlockSocket::operator<<(const Message& message)
{
    socklen_t tolen = sizeof(sockaddr);    
    logger << Logger::FINE << "Sending to " << inet_ntoa(to.sin_addr) << ":" << ntohs(to.sin_port) << "\n";
    int bytes = sendto(sock, &message.buffer.front(), message.buffer.size(), 0, (sockaddr*)&to, tolen);
    if (bytes < 0) {
        throw std::string(errmsg());
    } else if (bytes == 0) {
        throw std::string("Sent zero bytes on UDP socket");
    }
    
    // Return false if the size of the header plus the size reported in the
    // header isn't equal to the whole length of the packet that was sent
    Header* header = (Header*)&message.buffer.front();
    if (header->GetPacketLength() != (unsigned int)bytes) {
        logger << Logger::ERR << "Expected " << header->GetPacketLength() << " bytes, but sent " << bytes << "\n";  
        logger << Logger::ERR << "Data length: " << ntohl(header->length) << "\n";
        logger << Logger::ERR << "Header length: " << ntohs(header->offset) << "\n";
        throw std::string("Invalid packet length");
    }
	return *this;
}

BlockSocket& BlockSocket::operator>>(Address& address)
{
    address.ip_address = inet_ntoa(from.sin_addr);
	address.port = ntohs(from.sin_port);
	return *this;
}

BlockSocket& BlockSocket::operator<<(const Address& address)
{
	to.sin_addr.s_addr = inet_addr(address.ip_address.c_str());
	to.sin_port = htons(address.port);
	return *this;
}

void BlockSocket::close()
{
    if (sock != INVALID_SOCKET) {
#ifdef WINDOWS
	shutdown(sock, SD_BOTH);
        ::closesocket(sock);
#else
	::close(sock);
#endif
        sock = INVALID_SOCKET;
    }
}

void BlockSocket::set_timeout(long timeout)
{
    this->timeout = timeout;
}
