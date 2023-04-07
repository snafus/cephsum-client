#include "auth.h"

#include <openssl/hmac.h>
#include <openssl/evp.h>

#include <iostream>
#include <string>
#include <fstream>

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

bool answer_challenge(int sock, const std::string & authkey) {
    // message vars
    const int   MESSAGE_LENGTH {20};
    const char* CHALLENGE {"#CHALLENGE#"};
    const char* WELCOME   {"#WELCOME#"};
    const char* FAILURE   {"#FAILURE#"};
    const int CHALLENGE_LENGTH = MESSAGE_LENGTH + sizeof(CHALLENGE) / sizeof(CHALLENGE[0]);

    unsigned char *result = NULL;
    unsigned int resultlen = -1;
    int valread {0}; 
    char buffer[1024] = { 0 };

    for ( ssize_t i=0; i < 5; ++i) {
        valread = read(sock, buffer, 1024);
        if (valread > 0) break; // message recieved
        else if (valread < 0) {
            std::cerr << "Read Error at Auth: " << valread <<std::endl;
            return false;
        }
        // otherwise, just loop around until success or max attempts ... 
    }
    std::string sdata = std::string(buffer + 11, valread-11);
    // std::cout << "vread: " << valread << " " << sdata.c_str() << " " << sdata.size() << std::endl;

    result = HMAC(EVP_md5(), authkey.c_str(), authkey.size(), (const unsigned char*)sdata.c_str(), sdata.size(), result, &resultlen);
    // result = HMAC(EVP_md5(), authkey.c_str(), authkey.size(), data, datalen, result, &resultlen);
    ssize_t rc = send(sock, result, resultlen, 0);
    if (rc < 0) {
        std::cerr << "Read Error at Challenge send: " << valread <<std::endl;
    }

    valread = read(sock, buffer, CHALLENGE_LENGTH);
    if (valread <= 0) {
            std::cerr << "Read Error at Challenge welcome: " << valread <<std::endl;
            return false;
    }

    std::string resp(buffer, 9);

    // free(data);
    if (resp != "#WELCOME#") {
        std::cerr << "Authentication failure: " << resp << std::endl;
        return false;
    } else {
        // std::clog << "Connected ... " << std::endl;
    }
    return true;
}

bool read_secrets(const std::string &file, std::string &authkey) {
    // file with a single line, no comments, only holding the key
    std::ifstream ifile(file.c_str());
    if (!ifile) return false;
    
    std::getline(ifile,authkey);
    return true;
}


