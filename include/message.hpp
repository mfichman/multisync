#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include "fileinfo.hpp"

#include <iostream>
#include <vector>
#include <sstream>

#ifdef WINDOWS
#include <winsock2.h>
#else
#include <sys/types.h>
#include <netinet/in.h>
#endif

namespace Msync {
    class Message;
}

std::ostream& operator<<(std::ostream& stream, const Msync::Message& message);
std::istream& operator>>(std::istream& stream, Msync::Message& message);

namespace Msync {

enum MessageType {
    MESSAGE_TYPE_INFO,
    MESSAGE_TYPE_BLOCK,
    MESSAGE_TYPE_GETBLOCK,
    MESSAGE_TYPE_GETINFO,
    MESSAGE_TYPE_CGOODBYE,
    MESSAGE_TYPE_SGOODBYE,
    MESSAGE_TYPE_CHELLO
};

struct Header {
    unsigned int type;
    unsigned int length;
    unsigned int sender;
    unsigned short offset;
    
    unsigned int GetPacketLength() {
        return ntohl(length) + ntohs(offset);
    }
    
};

template <typename M>
struct MetadataHeader : Header {
    M metadata;
};


template <typename T>
struct Array {
    T* data;
    unsigned int length;
};


class Message {
public:

    enum Direction { OUTPUT, INPUT };

    friend class BlockSocket;
    friend std::ostream& ::operator<<(std::ostream& stream, const Message& message);
    friend std::istream& ::operator>>(std::istream& stream, Message& message);
    
    
    /**
     * Creates a new message.
     * @param reserved buffers
     */
    Message(unsigned int reserve);
	
	/**
	 * Creates a new message with now payload and not metdata.
	 * @param sender the message sender ID
	 * @param type the message type
	 */
	Message(unsigned int sender, unsigned int type);
    
    /**
     * Creates a new message with the given length and no metadata.
     * @param sender the message sender ID
     * @param type the message type
     * @param reserve the reserved buffer space in the payload
     */
    Message(unsigned int sender, unsigned int type, unsigned int reserve);
    
    /**
     * Creates a new message with the given length and no metadata.
     * @param sender the message sender ID
     * @param type the message type
     * @param text the message text
     */
    Message(unsigned int sender, unsigned int type, const std::string& text);
    
    /**
     * Creates a new message with no body, only metadata.
     * @param sender the message sender ID
     * @param type the message type
     * @param metadata the metadata.
     */
    template <typename M>
    Message(unsigned int sender, unsigned int type, const M& metadata);
    
    /**
     * Creates a new message with the given attributes.
     * @param sender the message sender ID
     * @param type the message type
     * @param reserve the reserved buffer space for the payload
     */
    template <typename M>
    Message(unsigned int sender, unsigned int type, const M& metadata, unsigned int reserve);
    
    /**
     * Creates a new message with the given attributes.
     * @param sender the message sender ID
     * @param type the message type  
     * @param text the message text
     */
    template <typename M>
    Message(unsigned int sender, unsigned int type, const M& metadata, const std::string& text);
    
    /**
     * Returns the message's metadata.
     * @return the message metadata
     */
    template <typename M>
    const M& get_metadata() const;
    
    /**
     * Returns the message's type
     * @return the message type
     */  
    unsigned int get_type() const;
    
    /**
     * Returns the message's length, not including the headers.
     * @return the message length
     */  
    unsigned int get_length() const;
    
    /**
     * Converts the body of this message into a string.
     * @return the string
     */
    const std::string get_text() const;
    
    /**
     * Converts the body of this message into an array.  The array points
     * to data in the message's internal buffer, so do not deallocate the
     * message while still using the array.
     * @return the array
     */
    template <typename T>
    const Array<T> get_array() const;
    
private:
    std::vector<char> buffer;
    const Direction direction;
};

template <typename M>
Message::Message(unsigned int sender, unsigned int type, const M& metadata) :
    buffer(sizeof(MetadataHeader<M>)),
    direction(OUTPUT)
{
    MetadataHeader<M>* header = (MetadataHeader<M>*)&buffer.front();
    header->type = htonl(type);
    header->sender = htonl(sender);
    header->length = 0;
    header->offset = htons(sizeof(MetadataHeader<M>));
    header->metadata = metadata;
}

template <typename M>
Message::Message(unsigned int sender, unsigned int type, const M& metadata, unsigned int reserve) :
    buffer(reserve + sizeof(MetadataHeader<M>)),
    direction(OUTPUT)
{
    MetadataHeader<M>* header = (MetadataHeader<M>*)&buffer.front();
    header->type = htonl(type);
    header->sender = htonl(sender);
    header->length = 0;
    header->offset = htons(sizeof(MetadataHeader<M>));
    header->metadata = metadata;
}

template <typename M>
Message::Message(unsigned int sender, unsigned int type, const M& metadata, const std::string& text) :
    buffer(text.length() + sizeof(MetadataHeader<M>)),
    direction(OUTPUT)
{
    MetadataHeader<M>* header = (MetadataHeader<M>*)&buffer.front();
    header->type = htonl(type);
    header->sender = htonl(sender);
    header->length = htonl(text.length());
    header->offset = htons(sizeof(MetadataHeader<M>));
    header->metadata = metadata;
    
    // Copy the string into the data portion of the buffer
    std::copy(text.begin(), text.end(), buffer.begin() + sizeof(MetadataHeader<M>));
}

template <typename M>
const M& Message::get_metadata() const
{
    MetadataHeader<M>* header = (MetadataHeader<M>*)&buffer.front();
    if (ntohs(header->offset) != sizeof(MetadataHeader<M>)) {
        std::stringstream ss;
        ss << "Message header size mismatch: expected ";
        ss << sizeof(MetadataHeader<M>) << " but received ";
        ss << ntohs(header->offset);
        throw std::string(ss.str());
    }
    return header->metadata;
}

template <typename T>
const Array<T> Message::get_array() const
{
    Header* header = (Header*)&buffer.front();
    if ((ntohl(header->length) % sizeof(T)) != 0) {
        throw std::string("Invalid buffer length for this type");
    }
    
    Array<T> array;
    array.data = (T*)(&buffer.front() + ntohs(header->offset));
    array.length = ntohl(header->length) / sizeof(T);
    
    return array;
}

}

#endif
