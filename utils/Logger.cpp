#include "Logger.h"

Logger::Logger()
    :m_Location(Location::DISABLED)
{
    if(!init()) std::cout << "Error! -> Logger::Logger" << std::endl;
}

Logger::Logger(const Location& location)
    :m_FileName("log.txt"),m_Location(location)
{
    if(!init()) std::cout << "Error! -> Logger::Logger" << std::endl;
}

Logger::Logger(const std::string &fileName,const Location& location)
    :m_FileName(fileName.substr((int)fileName.size()-4,4)!=".txt" ? fileName : fileName + ".txt"),
     m_Location(location)
{   
    if(!init()) std::cout << "Error! -> Logger::Logger" <<std::endl;
}

Logger::~Logger(){
    log("====================LOG FILE END======================");
    m_OutStream << '\n';
    m_OutStream.close();
}

bool Logger::init()
{
    /*
        Output Stream with appending into text file
    */
    if(m_Location == Location::DISABLED) return false;
    if(m_Location == Location::DATABASE) return false;   //Change later
    m_OutStream.open(m_FileName, std::ios_base::app);    
    if(!m_OutStream.is_open()) return false;

    m_ClassName = "Logger"; 
    log("====================LOG FILE START====================");
    return true;
}

bool Logger::log()
{
    return log("");
}

bool Logger::log(const std::string& str)
{
    return log({str});
}

bool Logger::log(const std::initializer_list<std::string> &list_str)
{
    /*
        Gets the current time and outputs in the log file 
    */
    if(m_Location == Location::DISABLED) return true;
    if(m_Location == Location::DATABASE) return true;     //Change later
    if(!m_OutStream.is_open()){
        std::cout << "Error-> Logger::log!" <<std::endl;
        return false;
    }
    time_t now = time(0); 
    tm* timeinfo = localtime(&now); 
    char timestamp[20]; 
    strftime(
        timestamp, 
        sizeof(timestamp), 
        "%Y-%m-%d %H:%M:%S", 
        timeinfo
        ); 
    
    m_OutStream  << timestamp << " -> ";
    for(int i=0; i<(int)list_str.size(); i++){
        for(auto const &c: *(list_str.begin()+i)){
            if(c!='\n')    
                m_OutStream << c;
        }
        if(i!=int(list_str.size())-1)
            m_OutStream << " || ";
    }
    m_OutStream << '\n';
    flush(m_OutStream);
    return true; 
}
