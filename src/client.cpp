#include "Client.h"
#include "common.h"

#include <fstream>

#include <cxxopts.hpp>


int main(int argc, char** argv)
{
    cxxopts::Options options("Client", "");
    options.add_options()
            ("v,verbosity", "Verbosity level", cxxopts::value<int>()->default_value("4"))
            ("manager_host", "RPC port", cxxopts::value<std::string>())
            ("input_graph", "Path to file containing graph", cxxopts::value<std::string>())
            ("h,help", "Print help and exit");
    options.parse_positional({"manager_host", "input_graph"});
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

    std::random_device rd;
    std::default_random_engine randomEngine(rd());
    std::normal_distribution<float> randomDistribution(0, sqrt(100.0));
    Eigen::Tensor<float, 1> input(100000);
    input = input.random([&randomEngine, &randomDistribution]{return randomDistribution(randomEngine);});


    std::ifstream fin(args["input_graph"].as<std::string>());

    auto output = client.processGraph({{0, input}}, fin, {5}).at(5);

    std::cout << std::get<Eigen::Tensor<float, 0>>(output);
}

