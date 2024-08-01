#include "Logger.h"

Logger::Logger()
    :m_FileName("log.txt")
{
    init();
}

Logger::Logger(const std::string &fileName)
    :m_FileName(fileName.substr((int)fileName.size()-4,4)!=".txt" ? fileName : fileName + ".txt")
{   
    init();
}

Logger::~Logger(){
    log("====================LOG FILE END======================\n");
}

bool Logger::init()
{
    /*
        Output Stream with appending
    */
    m_outstream.open(m_FileName, std::ios_base::app);         
    log("====================LOG FILE START====================");
    return true;
}

bool Logger::log()
{
    return log("");
}

bool Logger::log(const std::string& str)
{
    return log(str, "\n");
}

bool Logger::log(const std::string& str, const std::string& end)
{
    /*
        Gets the current time and outputs in the log file 
    */
    time_t now = time(0); 
    tm* timeinfo = localtime(&now); 
    char timestamp[20]; 
    strftime(
        timestamp, 
        sizeof(timestamp), 
        "%Y-%m-%d %H:%M:%S", 
        timeinfo
        ); 

    m_outstream  << timestamp << " -> " << str << end;
    flush(m_outstream);
    return true;
}
