#include "options.h"

#include <iostream>

#include <map>
#include <utility>
#include <algorithm>

#include "logging.h"

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
            else {
                std::cout << "Invalid mode provided; choose one of {cksum,stat,ping,health}" << std::endl;
                exit(EINVAL);
            }
            continue;
        }

        if ( (args[i] == "-a") || (args[i] == "--action") ){
            std::string action = args[++i];
            const std::vector<std::string> allowed_strings {"inget","fileonly","metaonly"};
            bool found_action {false};
            for (const auto & a: allowed_strings) {
                if (action != a) continue;
                found_action = true;
                break;
            }
            if (!found_action) {
                CERR("Invalid action provided; choose one of {inget,fileonly,metaonly}");
                exit(1);
            }
            options.m_action = action;
            continue;
        }
        if ( (args[i] == "-C") || (args[i] == "--cksum") ){
            options.m_cksalg = args[++i];
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