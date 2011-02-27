#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <iostream>

namespace Msync {

class Logger {
public:

    enum Level { FINEST, FINE, INFO, WARNING, ERR }; 

    /**
     * Creates a new logger with the given output stream.
     * @param output the output stream
     */
    Logger(std::ostream& output);
    
    /**
     * Logs a message to the output stream.
     * @param message the message to log
     */
    template <typename T>
    Logger& operator<<(const T object);
    
    /**
     * Changes the log level of messages written to the output stream.
     * @param level the log level
     */
    Logger& operator<<(const Level& level);
    
    
    /**
     * Sets the level of the logger.  All messages below the given level will
     * be discarded.
     * @param level the log level
     */
    void set_level(const Level& level);
    
    static Logger Default;
    
private:
    std::ostream& output; 
    Level output_level;
    Level message_level;  
};


template <typename T>
Logger& Logger::operator<<(const T object)
{
    if (message_level >= output_level) {
        output << object;
        output.flush();
    }
    return *this;
}

}

#endif
