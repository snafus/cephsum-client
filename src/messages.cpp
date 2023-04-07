#include "messages.h"
#include <iostream>

#include <arpa/inet.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"




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
    // std::clog << msg << std::endl;
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

    return true;
}


bool connect(const std::string & host, int port, int& sock, int& client_fd) {
    struct sockaddr_in serv_addr;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        std::cerr << "Socket creation error" << std::endl;
        return false;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(port);
    
    if (inet_pton(AF_INET, host.c_str(), &serv_addr.sin_addr)  <= 0) {
        std::cerr << "Invalid address/ Address not supported" << std::endl;
        return false;
    }
    if ((client_fd = connect(sock, (struct sockaddr*)&serv_addr,
                   sizeof(serv_addr))) < 0) {
        std::cerr << "Connection Failed" << std::endl;
        return false;
    }

    struct timeval timeout;      
    timeout.tv_sec = 20;
    timeout.tv_usec = 1000;

    if (setsockopt (sock, SOL_SOCKET, SO_RCVTIMEO, &timeout,
                sizeof timeout) < 0)
        printf("setsockopt failed\n");

    if (setsockopt (sock, SOL_SOCKET, SO_SNDTIMEO, &timeout,
                sizeof timeout) < 0)
        printf("setsockopt failed\n");

    return true;
}


bool create_message(const Options& opt, rapidjson::Document & msg) {
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
         return false;
    }
    return true;
}
