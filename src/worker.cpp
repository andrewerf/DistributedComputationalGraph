#include "Worker.h"

#include <cxxopts.hpp>


int main(int argc, char** argv)
{
    cxxopts::Options options("Worker", "");
    options.add_options()
            ("v,verbosity", "Verbosity level", cxxopts::value<int>()->default_value("4"))
            ("kafka_host", "Kafka host", cxxopts::value<std::string>())
            ("redis_host", "Redis host", cxxopts::value<std::string>())
            ("manager_host", "RPC port", cxxopts::value<std::string>())
            ("h,help", "Print help and exit");
    options.parse_positional({"kafka_host", "redis_host", "manager_host"});
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
        {"enable.auto.commit", "true"}
    });


    Worker worker(properties, args["manager_host"].as<std::string>(), args["redis_host"].as<std::string>());
    worker.listen();
}

