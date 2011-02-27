#include "message.hpp"
#include <iostream>
#include <cerrno>
#include <cstring>

using namespace Msync;

Message::Message(unsigned int reserve) :
    buffer(reserve + sizeof(Header)),
    direction(INPUT)
{
    Header* header = (Header*)&buffer.front();
    header->type = 0;
    header->sender = 0;
    header->length = 0;
    header->offset = 0;
}

Message::Message(unsigned int sender, unsigned int type) :
    buffer(sizeof(Header)),
    direction(OUTPUT)
{
    Header* header = (Header*)&buffer.front();
    header->type = htonl(type);
    header->sender = htonl(sender);
    header->length = 0;
    header->offset = ntohs(sizeof(Header));
}

Message::Message(unsigned int sender, unsigned int type, unsigned int reserve) :
    buffer(reserve + sizeof(Header)),
    direction(OUTPUT)
{
    Header* header = (Header*)&buffer.front();
    header->type = htonl(type);
    header->sender = htonl(sender);
    header->length = 0;
    header->offset = ntohs(sizeof(Header));
}

Message::Message(unsigned int sender, unsigned int type, const std::string& text) :
    buffer(text.length() + sizeof(Header)),
    direction(OUTPUT)
{
    Header* header = (Header*)&buffer.front();
    header->type = htonl(type);
    header->sender = htonl(sender);
    header->length = htonl(text.length());
    header->offset = ntohs(sizeof(Header));
    
    // Copy the string into the data portion of the buffer
    std::copy(text.begin(), text.end(), buffer.begin() + sizeof(Header));
}

unsigned int Message::get_type() const
{
    // Return the type from the header portion
    Header* header = (Header*)&buffer.front();
    return ntohl(header->type);
}

unsigned int Message::get_length() const
{
    // Return the length of the data portion of the message
    Header* header = (Header*)&buffer.front();
    return ntohl(header->length);
}

const std::string Message::get_text() const
{
    // Convert the data portion of the packet into a string
    Header* header = (Header*)&buffer.front();
    return std::string(buffer.begin() + ntohs(header->offset), buffer.end());
}

std::ostream& operator<<(std::ostream& stream, const Message& message)
{
    // Write the data portion (not the header) to the output stream
    Header* header = (Header*)&message.buffer.front();
    stream.write(&message.buffer.front() + ntohs(header->offset), message.buffer.size() - ntohs(header->offset));
    
    if (stream.fail()) {
        throw std::string("Could not write data block to file: ") + strerror(errno);
    }
    
    return stream;
}

std::istream& operator>>(std::istream& stream, Message& message)
{
    Header *header = (Header*)&message.buffer.front();
    
    // Only read if the buffer doesn't already have data in it
    if (header->length == 0) {
    
        // Read into the data portion of the packet
        stream.read(&message.buffer.front() + ntohs(header->offset), message.buffer.size() - ntohs(header->offset));
        header->length = htonl(stream.gcount());
        
        // Trim the buffer to match the size of the read data
        message.buffer.resize(ntohs(header->offset) + stream.gcount());
        
        if (stream.bad()) {
            throw std::string("Could not read data block from file: ") + strerror(errno);   
        }
    }
    
    return stream;
}
