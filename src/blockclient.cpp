#include "blockclient.hpp"
#include "blockinfo.hpp"
#include "fileinfo.hpp"
#include "blockclient.hpp"


#ifdef WINDOWS
#define getpid() GetCurrentProcessId()
#define sleep(x) Sleep(x * 1000)
#else
#include <unistd.h>
#endif

#define BLOCKSIZE 1024

using namespace Msync;

BlockClient::BlockClient(const std::string& group, unsigned short port, Logger& logger) :
    info(getpid()),
    logger(logger),
    socket(group, port)
{
    logger << Logger::FINE << "Host ID is " << info.get_id() << "\n";
	handlers[MESSAGE_TYPE_INFO] = &BlockClient::handle_info;
	handlers[MESSAGE_TYPE_BLOCK] = &BlockClient::handle_block;
	handlers[MESSAGE_TYPE_SGOODBYE] = &BlockClient::handle_sgoodbye;
}

void BlockClient::start()
{
    socket.open();
    
    logger << Logger::INFO << "Sending hello message\n";
    message_queue.push_back(Message(id, MESSAGE_TYPE_CHELLO, info));   
    
    while (true) {
        select(-1);
    }
}

void BlockClient::select(long timeout)
{
    // Read/write any oustanding messages
    BlockSocket::Status status = socket.select(timeout, !message_queue.empty());
    if (status == BlockSocket::READ || status == BlockSocket::BOTH) {
        process_message();
    } 
    if (status == BlockSocket::WRITE || status == BlockSocket::BOTH) {
        socket << message_queue.front();
        message_queue.pop_front();
    }
}

void BlockClient::process_message()
{    
    Message message(4096);
	Address address;
    socket >> message >> address;

	typedef std::map<unsigned int, message_handler> handler_map;
	handler_map::iterator i = handlers.find(message.get_type());

	if (i != handlers.end()) {
		message_handler handler = i->second;
		(this->*handler)(message, address);
	} else {
		logger << Logger::WARNING << "Unknown message type\n";
	}
}

void BlockClient::handle_info(const Message& message, const Address& address)
{
	const FileInfo& info = message.get_metadata<FileInfo>();
    SyncStatus& status = get_sync_status(info, address);
    logger << Logger::INFO << "Received file information for " << message.get_text() << "\n";
    status.set_path(message.get_text());

	if (status.sync_complete()) {
        sync_set.erase(info);
    }
}

void BlockClient::handle_block(const Message& message, const Address& address)
{
	// Look up the current status of the file by using the file
    // info object
    const BlockInfo& block = message.get_metadata<BlockInfo>();
    const FileInfo& info = block.get_file_info();
    SyncStatus& status = get_sync_status(info, address);
    logger << Logger::INFO << "Received block #" << block << " (" << message.get_length() << " bytes)\n";
    status.write_block(block, message);

	if (status.sync_complete()) {
        sync_set.erase(info);
    }
}

void BlockClient::handle_sgoodbye(const Message& message, const Address& address)
{
    // Get the list of clients the server has marked as finished
    const FileInfo& info = message.get_metadata<FileInfo>();
	SyncStatus& status = get_sync_status(info, address);
	logger << Logger::INFO << "Received server goodbye\n";
			
	if (status.transfer_complete()) {
		Array<unsigned> clients = message.get_array<unsigned>();
		for(unsigned i = 0; i < clients.length; i++) {
			if (clients.data[i] == id) {
				status.set_goodbye_received();
				message_queue.push_back(Message(id, MESSAGE_TYPE_CGOODBYE));
			}
		}
	} else {
		// TODO: Send a request for missing blocks!!
	}

	if (status.sync_complete()) {
        sync_set.erase(info);
    }
}

SyncStatus& BlockClient::get_sync_status(const FileInfo& info, const Address& address)
{
    std::map<FileInfo, SyncStatus>::iterator i = sync_set.find(info);
    if (i == sync_set.end()) {
        i = sync_set.insert(i, std::pair<FileInfo, SyncStatus>(info, SyncStatus(info, address)));
    }
    return i->second;   
}
