#include <iostream>
#include <string>
#include <fstream>

#include <sstream>
#include <iomanip>
#include <vector>
#include <map>
#include <utility>
#include <algorithm>

#include <openssl/hmac.h>
#include <openssl/evp.h>

#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

#include "auth.h"
#include "messages.h"
#include "options.h"
#include "display.h"
#include "logging.h"


int main(int argc, char* argv[]) {
    Options opt = argparse(argc, argv);

    // check if help options was specified
    if (opt.m_wantsHelp) {
        display_help();
        return 0;
    }

    // read the secrets in and check we did
    if (!read_secrets(opt.m_secretsFile, opt.m_authkey) ){
        CERR("Error reading in the secrets file");
        return EINVAL;
    }

    // attempt to open a connection to the server, and respond to the challenge
    if (opt.m_verbose) {
        CLOG("Attempting to connect to " 
                  << opt.m_host << ":" << opt.m_port);
    }
    int sock = 0, valread, client_fd;
    bool connected{false};
    for (size_t attempt=0; attempt<5; ++attempt) {
        if (!connect(opt.m_host, opt.m_port, sock, client_fd)) {
            CERR("Unable to connect");
            usleep(1000000*(1+attempt));
            continue;
        }
        if (!answer_challenge(sock, opt.m_authkey)){
            close(client_fd);
            usleep(1000000*(1+attempt));
            continue;
        }
        connected = true;
        break;
    }
    if (!connected) {
        CERR("Could not connect / authenticate to host" 
                  << opt.m_host << ":" << opt.m_port);
        return EIO; 
    }

    // create the message to send
    rapidjson::Document msg;
    if (!create_message(opt, msg)) {
        CERR("Could not create the message to send");
        close(client_fd);
        return EINVAL;
    }
    // send message
    msg_send(sock, msg);

    // await response
    rapidjson::Document response;
    while (true) { 
        if (! msg_recv(sock, response) ) {
            CERR("Error recieving response");
            close(client_fd);
            return EINVAL;
        }
        if (opt.m_verbose) {
            std::clog << ".";
        }
        if (response["msg"] != "alive") break;
    }
    if (opt.m_verbose) {
        std::clog << std::endl;
    }
    close(client_fd);

    int status = response["status"].GetInt();
    if (status != 0) {
        rapidjson::StringBuffer buffer;
        rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
        response.Accept(writer);
        CERR("Server returned an error: " << buffer.GetString());
        return 1; 
    }


    // present the results and get the return code
    int rc = presentResults(opt, msg, response);
    return rc;
}


