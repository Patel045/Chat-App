#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>   // for including ofstream
#include <string>    // for dealing with names
#include <ctime>

class Logger{
private:
    const std::string m_FileName;   //do we need static?
    std::ofstream m_outstream;
    bool init();
protected:
    std::string m_ClassName;  //inherit to children
public:
    Logger();
    Logger(const std::string&);
    ~Logger();
    bool log();
    bool log(const std::string&);
    bool log(const std::string&, const std::string&);
};

#endif