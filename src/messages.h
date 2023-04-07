#ifndef MESSAGES_H
#define MESSAGES_H

#include <string>
#include "options.h"
#include "rapidjson/document.h"


bool connect(const std::string & host, int port, int& sock, int& client_fd);

bool msg_recv(int sock, rapidjson::Document & d);
bool msg_send(int sock, rapidjson::Document & d);

bool create_message(const Options&opt, rapidjson::Document & d);

#endif 