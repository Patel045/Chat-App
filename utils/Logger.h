#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <fstream>   // for including ofstream
#include <string>    // for dealing with names
#include <ctime>     // for writing time 

enum Location{
    DISABLED,
    TEXT_FILE,
    DATABASE
};

class Logger{
private:
    const std::string m_FileName;   //do we need static?
    std::ofstream m_OutStream;
    const Location m_Location;
    bool init();
protected:
    std::string m_ClassName;  //inherit to children
public:
    Logger();
    Logger(const Location&);
    Logger(const std::string&, const Location&);
    ~Logger();
    bool log();
    bool log(const std::string&);
    bool log(const std::initializer_list<std::string>&);
};

#endif