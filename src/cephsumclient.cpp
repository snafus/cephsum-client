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
        
    }
    #warning
    valread = read(sock, buffer, CHALLENGE_LENGTH);

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

bool msg_send(int sock, rapidjson::Document & d) {
    // 3. Stringify the DOM
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    d.Accept(writer);
    const std::string strLength = "XXXX";

    std::string msg = strLength + buffer.GetString();
    int mlen = msg.size() - 4;

    unsigned char *data = (unsigned char *)strdup(msg.c_str());
    data[0] = (mlen >> 24) & 0xFF;
    data[1] = (mlen >> 16) & 0xFF;
    data[2] = (mlen >>  8) & 0xFF;
    data[3] = (mlen >>  0) & 0xFF;
    std::clog << msg << std::endl;
    ssize_t rc = send(sock, data, mlen+4, 0);
    if (rc < 0) {
        std::cerr << "Error on send: " << rc << std::endl;
    }
    free(data);
    return true;
}

bool msg_recv(int sock, rapidjson::Document & d) {
    int valread {0}; 
    char buffer[1024] = { 0 };

    // get the size of the message
    valread = read(sock, buffer, 4);
    if (valread == 0) {
        std::clog << "0 bytes read" << std::endl;
        return false;
    }
    int mrecv = 0;
    mrecv |= uint8_t(buffer[0]) << 24;
    mrecv |= uint8_t(buffer[1]) << 16;
    mrecv |= uint8_t(buffer[2]) <<  8;
    mrecv |= uint8_t(buffer[3]) <<  0;

    // read the message
    valread = read(sock, buffer, mrecv);
    if (valread == 0) {
        std::clog << "0 bytes read" << std::endl;
        return false;
    }

    std::string msg_rec(buffer, mrecv);
    // std::clog << msg_rec << std::endl;

    // d.Clear();
    d.SetObject();
    d.Parse(msg_rec.c_str());
    rapidjson::StringBuffer sbuffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(sbuffer);
    d.Accept(writer);

    // std::clog << d["msg"].GetString() << std::endl;
    // std::clog << d["status"].GetInt() << std::endl;
    // std::clog << d["status_message"].GetString() << std::endl;
    // std::clog << d["details"]["response"].GetString() << std::endl;
    // std::clog << d["details"]["digest"].GetString() << std::endl;
    return true;
}

bool connect(const std::string & host, int port, int& sock, int& client_fd) {
    struct sockaddr_in serv_addr;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return false;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);

    if (inet_pton(AF_INET, host.c_str(), &serv_addr.sin_addr)  <= 0) {
        printf("\nInvalid address/ Address not supported \n");
        return false;
    }
    if ((client_fd = connect(sock, (struct sockaddr*)&serv_addr,
                   sizeof(serv_addr))) < 0) {
        printf("\nConnection Failed \n");
        return false;
    }

    #warning
    struct timeval timeout;      
    timeout.tv_sec = 10;
    timeout.tv_usec = 1000;

    if (setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, &timeout,
                sizeof timeout) < 0)
        printf("setsockopt failed\n");

    if (setsockopt (sock, SOL_SOCKET, SO_SNDTIMEO, &timeout,
                sizeof timeout) < 0)
        printf("setsockopt failed\n");

    return true;
}

struct Options {
    enum MODE {CKSUM=0, STAT, PING, HEALTH, WAIT};
    std::string m_mode {"cksum"};
    MODE m_emode {CKSUM};
    std::vector<std::string> m_args; 

    std::string m_secretsFile {"/etc/xrootd/cephsum-secrets"};
    std::string m_authkey;
    std::string m_host {"127.0.0.1"};
    int m_port {6000};

    bool m_wantsHelp {false};
    bool m_verbose {false};

    // checksum options
    std::string m_action {"inget"};
    std::string m_cksalg {"adler32"};
};

bool read_secrets(const std::string &file, std::string &authkey) {
    // file with a single line, no comments, only holding the key
    std::ifstream ifile(file.c_str());
    std::getline(ifile,authkey);
    return true;
}

Options argparse(int argc, char* argv[]) {
    if (argc > 64) {
        throw std::runtime_error("Too many input parameters!");
    }
    const std::vector<std::string> args(argv + 1, argv + argc);
    Options options; 

    size_t i =0;
    for (; i < args.size(); ++i) {
        if ( (args[i] == "-h") || (args[i] == "--help") ){
            options.m_wantsHelp = true;
            continue;
        }
         if ( (args[i] == "-d") || (args[i] == "--verbose") ){
            options.m_verbose = true;
            continue;
        }       

        if ( (args[i] == "-p") || (args[i] == "--port") ){
            options.m_port = stoi(args[++i]);
            continue;
        }

        if ( (args[i] == "-s") || (args[i] == "--secrets") ){
            options.m_secretsFile = args[++i];
            read_secrets(options.m_secretsFile, options.m_authkey);
            continue;
        }

        if ( (args[i] == "-m") || (args[i] == "--mode") ){
            std::string mode = args[++i];
            options.m_mode = mode;
            if (mode == "cksum") options.m_emode = Options::CKSUM;
            else if (mode == "stat")   options.m_emode = Options::STAT;
            else if (mode == "ping")   options.m_emode = Options::PING;
            else if (mode == "health") options.m_emode = Options::HEALTH;            
            else if (mode == "wait")   options.m_emode = Options::WAIT;

            continue;
        }

        if ( (args[i] == "-a") || (args[i] == "--action") ){
            options.m_action = args[++i];
            continue;
        }
        if ( (args[i] == "-C") || (args[i] == "--cksum") ){
            options.m_cksalg = args[++i];
            continue;
        }

        if ((args[i] == "--port") ){
            options.m_port =  stoi(args[++i]);
            continue;
        }
        if ((args[i] == "--host") ){
            options.m_host =  args[++i];
            continue;
        }


        // if no option found, assume that we are at the final set of args
        break;
    } // for 
    // add 
    for (; i < args.size(); ++i)
        options.m_args.push_back(args[i]);
    // std::copy(argv, argv + argc, std::ostream_iterator<char *>(std::cout, "\n"));
    return options;
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



int main(int argc, char* argv[]) {
    Options opt = argparse(argc, argv);

    // std::string path = opt.m_args.at(0);

    // const std::string secret {"secret password"};

    int sock = 0, valread, client_fd;
    bool connected{false};
    for (size_t attempt=0; attempt<5; ++attempt) {
        if (!connect(opt.m_host, opt.m_port, sock, client_fd)) {
            std::cerr  << "Unable to connect" << std::endl;
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
        throw std::runtime_error("Could not connect / authenticate");
    }


    rapidjson::Document msg;
    msg.SetObject();
    rapidjson::Value v_msg;
    v_msg.SetString(opt.m_mode.c_str(), opt.m_mode.size(), msg.GetAllocator());
    msg.AddMember("msg", v_msg, msg.GetAllocator());

    switch (opt.m_emode) {
            case Options::MODE::STAT:
            {
                rapidjson::Value v_path;
                v_path.SetString   (opt.m_args.at(0).c_str(), opt.m_args.at(0).size(), msg.GetAllocator());
                msg.AddMember("path",    v_path,    msg.GetAllocator());
            }
        case Options::MODE::CKSUM:
            {
            rapidjson::Value v_path, v_action, v_algtype;
            v_path.SetString   (opt.m_args.at(0).c_str(), opt.m_args.at(0).size(), msg.GetAllocator());
            v_action.SetString (opt.m_action.c_str(), opt.m_action.size(),  msg.GetAllocator());
            v_algtype.SetString(opt.m_cksalg.c_str(), opt.m_cksalg.size(), msg.GetAllocator());
            msg.AddMember("path",    v_path,    msg.GetAllocator());
            msg.AddMember("action",  v_action,  msg.GetAllocator());
            msg.AddMember("algtype", v_algtype, msg.GetAllocator());
            }
            break;
        case Options::MODE::HEALTH:
            break;
        case Options::MODE::PING:
            break;
        case Options::MODE::WAIT:
            {
            rapidjson::Value v_delay;
            v_delay.SetInt( atoi(opt.m_args.at(0).c_str()) );
            msg.AddMember("delay", v_delay, msg.GetAllocator());
            }
            break;
        default:
         throw std::runtime_error("Invalid mode specified");
    }

    msg_send(sock, msg);

    rapidjson::Document response;
    while (true) { 
        if (! msg_recv(sock, response) ) {
            std::cerr << "Error recieving response" << std::endl;
            close(client_fd);
            return EINVAL;
        }
        if (response["msg"] != "alive") break;
    }

    close(client_fd);

    // present the results 
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
         throw std::runtime_error("Invalid mode specified");
    }
    // shouldn't get to this point 
    return 0;
}


