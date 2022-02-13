#ifndef DISTRIBUTEDCOMPUTATIONALGRAPH_WORKER_H
#define DISTRIBUTEDCOMPUTATIONALGRAPH_WORKER_H

#include "Node.h"

#include <kafka/KafkaConsumer.h>

#include <rpc/client.h>

#include <redis++.h>


class Worker
{
public:
    Worker(const kafka::Properties &consumerProps, const std::string &managerRpcHost, const std::string &redisUri);

    void listen();

    void processNode(const Node &node);

private:
    kafka::clients::KafkaConsumer consumer;
    rpc::client rpcClient;
    sw::redis::Redis redisServer;
};

#endif //DISTRIBUTEDCOMPUTATIONALGRAPH_WORKER_H
