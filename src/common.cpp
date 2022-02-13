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
