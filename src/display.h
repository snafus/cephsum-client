#ifndef DISPLAY_H
#define DISPLAY_H
#include "options.h"
#include "rapidjson/document.h"

// CLI help message
void display_help();

// outputs from each actions, with a return code
int display_ping  (rapidjson::Document & d);
int display_wait  (rapidjson::Document & d);
int display_health(rapidjson::Document & d);
int display_cksum (rapidjson::Document & d);
int display_stat  (rapidjson::Document & d);

int presentResults(const Options& opt, rapidjson::Document &response);

#endif