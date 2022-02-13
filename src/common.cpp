#include "common.h"

#include <plog/Appenders/ColorConsoleAppender.h>
#include <plog/Formatters/TxtFormatter.h>
#include <plog/Init.h>
#include <plog/Log.h>

#include <kafka/KafkaClient.h>


void KafkaLogger(int level, const char* /*filename*/, int /*lineno*/, const char* msg)
{
    PLOG(static_cast<plog::Severity>(level)) << msg;
}

void setUpLogger(plog::Severity severity)
{
    static plog::ColorConsoleAppender<plog::TxtFormatter> consoleAppender;

    plog::init(severity, &consoleAppender);
    kafka::clients::KafkaClient::setGlobalLogger(KafkaLogger);
}

std::string extractHostname(const std::string &host)
{
    auto pos = host.find(':');
    if(pos == std::string::npos)
        return host;
    else
        return host.substr(0, pos);
}

int extractRpcPort(const std::string &host)
{
    auto pos = host.find(':');
    if(pos == std::string::npos)
        return 8080; /* rpc::constants::DEFAULT_PORT */
    else
        return std::stoi(host.substr(pos + 1));
}
