#include "display.h"

#include <iostream>
#include <string>
#include <fstream>

#include <sstream>
#include <iomanip>
#include <vector>
#include <map>
#include <utility>
#include <algorithm>

#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"


void display_help() {
    std::stringstream ss;
    ss << "Usage: \n\t" << "cephsumclient [-h] [-p] [--host] [-s] [-d] --mode <mode> [--action <inget ] [--cksum <opts>]";
    ss << "\n\t\t -h | --help   : display this help and exit";
    ss << "\n\t\t -d| --verbose : extra logging info";
    ss << "\n\t\t -p | --port   : port number to connect to";
    ss << "\n\t\t --host        : host to connect to";
    ss << "\n\t\t -m| --mode    : what mode to run: cksum,stat,ping,health";
    ss << "\n\t\t -a| --action  : For cksum, which action to perform: inget,fileonly,metaonly";
    ss << "\n\t\t -C| --cksum   : Additional checksum options; see XRootD documentation for details";
    ss << "\n\t\t -s| --secrets : location of the secrets file with the authorization string";
    
    std::cout << ss.str() << std::endl;
}

int display_ping(rapidjson::Document & d) {
    if (d["status"] != 0) {
        std::cerr << "Error with ping" << std::endl;
        return 1;
    }
    auto & dd = d["details"];
    std::cout << dd["response"].GetString() << std::endl;
    return 0;
}

int display_wait(rapidjson::Document & d) {
    if (d["status"] != 0) {
        std::cerr << "Error with wait" << std::endl;
        return 1;
    }
    auto & dd = d["details"];
    std::cout << "Waited: " << dd["delay"].GetFloat() << "s" << std::endl;
    return 0;
}

int display_health(rapidjson::Document & d) {
    if (d["status"] != 0) {
        std::cerr << "Error with wait" << std::endl;
        return 1;
    }
    auto & dd = d["details"];
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    dd.Accept(writer);

    std::cout << buffer.GetString() << std::endl;
    return 0;
}

int display_cksum(rapidjson::Document & d) {
    auto & dd = d["details"];
    int status = d["status"].GetInt();
    if (status == 0) {
        std::cout << dd["digest"].GetString() << std::endl;
    } else {
        std::cerr << dd["error"].GetString() << std::endl;
    }
    return status;
}

int display_stat(rapidjson::Document & d) {
    auto & dd = d["details"];
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    int status = d["status"].GetInt();
    if (status == 0) {
        dd["stat"].Accept(writer);
        std::cout << buffer.GetString() << std::endl;
    } else {
        // dd["error"].Accept(writer);
        std::cerr << dd["error"].GetString() << std::endl;
    }
    return status;
}


int presentResults(const Options& opt, rapidjson::Document &response) {

    switch (opt.m_emode) {
        case Options::MODE::PING:
            return display_ping(response);
        case Options::MODE::WAIT:        
            return display_wait(response);
        case Options::MODE::HEALTH:
            return display_health(response);
        case Options::MODE::CKSUM:
            return display_cksum(response);
        case Options::MODE::STAT:
            return display_stat(response);
        default:
            std::cerr << "Invalid mode specified at printout" << std::endl;
            return EINVAL;
    }


}
