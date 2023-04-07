#ifndef OPTIONS_H
#define OPTIONS_H

#include <string>
#include <vector>

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

Options argparse(int argc, char* argv[]);

#endif 