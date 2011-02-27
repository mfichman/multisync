#include "logger.hpp"
#include <ctime>
#include <cstring>

using namespace Msync;


Logger Logger::Default(std::cout);

Logger::Logger(std::ostream& output) :
    output(output),
    output_level(INFO),
    message_level(INFO)
{    
}

Logger& Logger::operator<<(const Level& level)
{
    message_level = level;
    
    if (message_level >= output_level) {
        time_t rawtime;
        time(&rawtime);
        char* timestring = asctime(localtime(&rawtime));
        timestring[strlen(timestring) - 1] = '\0';
        output << timestring;
    
        switch (level) {
            case FINEST: output << " [FINEST] "; break;
            case FINE: output << " [FINE] "; break;
            case INFO: output << " [INFO] "; break;
            case WARNING: output << " [WARNING] "; break;
            case ERR: output << " [ERROR] "; break;
        }
    }
    
    return *this;
}

void Logger::set_level(const Level& level)
{
    output_level = level;
}
