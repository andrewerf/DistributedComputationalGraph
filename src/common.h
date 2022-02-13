#ifndef DISTRIBUTEDCOMPUTATIONALGRAPH_COMMON_H
#define DISTRIBUTEDCOMPUTATIONALGRAPH_COMMON_H

#include <plog/Severity.h>


void KafkaLogger(int level, const char* /*filename*/, int /*lineno*/, const char* msg);

void setUpLogger(plog::Severity severity);


#endif //DISTRIBUTEDCOMPUTATIONALGRAPH_COMMON_H
