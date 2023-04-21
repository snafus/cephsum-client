#ifndef LOGGING_H
#define LOGGING_H

#include <iostream>
#include <sstream> 
#include <iomanip>

// very simple approach to logging 

/// timestamp output for logging messages
static std::string ts() {
    std::time_t t = std::time(nullptr);
    char mbstr[50];
    std::strftime(mbstr, sizeof(mbstr), "CEPHSUMCLIENT-%y%m%d %H:%M:%S ", std::localtime(&t));
    return std::string(mbstr);
}

#define CLOG(x) {std::stringstream _bs;  _bs << ts() << x; std::clog << _bs.str() << std::endl;}
#define CERR(x) {std::stringstream _bs;  _bs << ts() << x; std::cerr << _bs.str() << std::endl;}


#endif