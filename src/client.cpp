#include "Client.h"
#include "common.h"

#include <cxxopts.hpp>


int main(int argc, char** argv)
{
    cxxopts::Options options("Client", "");
    options.add_options()
            ("v,verbosity", "Verbosity level", cxxopts::value<int>()->default_value("4"))
            ("manager_host", "RPC port", cxxopts::value<std::string>())
            ("h,help", "Print help and exit");
    options.parse_positional({"manager_host"});
    cxxopts::ParseResult args;

    try {
        args = options.parse(argc, argv);
    }
    catch(std::exception &e)
    {
        std::cout << options.help() << std::endl;
        exit(-1);
    }
    if(args.count("help") > 0)
    {
        std::cout << options.help() << std::endl;
        exit(0);
    }

    setUpLogger(static_cast<plog::Severity>(args["verbosity"].as<int>()));

    Client client(args["manager_host"].as<std::string>());
    client.speak();
}

