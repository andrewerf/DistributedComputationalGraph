#ifndef DISTRIBUTEDCOMPUTATIONALGRAPH_MANAGER_H
#define DISTRIBUTEDCOMPUTATIONALGRAPH_MANAGER_H

#include "Node.h"

#include <kafka/KafkaProducer.h>

#include <rpc/server.h>

#include <redis++.h>


class Manager
{
    Manager(const kafka::Properties &producerProps, int rpcPort, int rpcThreads, const std::string &redisUri);

    void addGraph(TID id);
    TID addGraphUnique();
    void addNode(const Node &node);

private:
    kafka::clients::KafkaProducer producer;
    rpc::server rpcServer;
    sw::redis::Redis redisServer;

    std::mutex addGraphMutex;
};

#endif //DISTRIBUTEDCOMPUTATIONALGRAPH_MANAGER_H
