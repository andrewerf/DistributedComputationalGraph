#include "Manager.h"

#include <cxxopts.hpp>


int main(int argc, char** argv)
{
    cxxopts::Options options("Manager", "");
    options.add_options()
            ("v,verbosity", "Verbosity level", cxxopts::value<int>()->default_value("4"))
            ("kafka_host", "Kafka host", cxxopts::value<std::string>())
            ("redis_host", "Redis host", cxxopts::value<std::string>())
            ("rpc_port", "RPC port", cxxopts::value<int>()->default_value(std::to_string(rpc::constants::DEFAULT_PORT)))
            ("rpc_threads", "RPC threads count", cxxopts::value<int>()->default_value("6"))
            ("h,help", "Print help and exit");
    options.parse_positional({"kafka_host", "redis_host"});
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

    kafka::Properties properties({
        {"bootstrap.servers", args["kafka_host"].as<std::string>()},
        {"enable.idempotence", "true"}
    });

    Manager manager(properties, args["rpc_port"].as<int>(), args["rpc_threads"].as<int>(), args["redis_host"].as<std::string>());

    while (true)
        std::this_thread::sleep_for(std::chrono::seconds(10));
}

