#ifndef DISTRIBUTEDCOMPUTATIONALGRAPH_COMMON_H
#define DISTRIBUTEDCOMPUTATIONALGRAPH_COMMON_H

#include <plog/Severity.h>

#include <string>


void KafkaLogger(int level, const char* /*filename*/, int /*lineno*/, const char* msg);

void setUpLogger(plog::Severity severity);


std::string extractHostname(const std::string &host);

int extractRpcPort(const std::string &host);



#endif //DISTRIBUTEDCOMPUTATIONALGRAPH_COMMON_H
