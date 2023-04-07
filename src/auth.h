#ifndef AUTH_H
#define AUTH_H

#include <string>


/// 
///
bool answer_challenge(int sock, const std::string & authkey);

/// Read the secret string from the input file
/// returns true if succesful 
bool read_secrets(const std::string &file, std::string &authkey);



#endif 
