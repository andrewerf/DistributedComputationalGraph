#ifndef DISTRIBUTEDCOMPUTATIONALGRAPH_MANAGER_H
#define DISTRIBUTEDCOMPUTATIONALGRAPH_MANAGER_H

#include "Node.h"
#include "Graph.h"

#include <shared_mutex>

#include <kafka/KafkaProducer.h>

#include <rpc/server.h>

#include <redis++.h>


class Manager
{
public:
    Manager(const kafka::Properties &producerProps, int rpcPort, int rpcThreads, const std::string &redisUri);

    void addGraph(TID id);
    TID addGraphUnique();
    void addNode(const Node &node);
    void setInputValue(TID graphId, TID nodeId, const std::string &data, MetaData md);
    void setNodeComputed(TID graphId, TID nodeId);
    void sendNode(const Node &node);
    std::string getData(TID graphId, TID nodeId);
    MetaData getMetaData(TID graphId, TID nodeId);
    bool isComputed(TID graphId, TID nodeId);

private:
    kafka::clients::KafkaProducer producer;
    rpc::server rpcServer;
    sw::redis::Redis redisServer;

    TID maxGraphId = 0;
    std::unordered_map<TID, std::unordered_set<TID>> nodeIds;
    std::unordered_map<TID, Graph> graphs;
    std::shared_mutex graphsMutex;
};

#endif //DISTRIBUTEDCOMPUTATIONALGRAPH_MANAGER_H
