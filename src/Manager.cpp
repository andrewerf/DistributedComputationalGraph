#include "Manager.h"

#include <kafka/AdminClient.h>

#include <plog/Log.h>

#include <rpc/msgpack.hpp>
#include <rpc/this_handler.h>


Manager::Manager(const kafka::Properties &producerProps, int rpcPort, int rpcThreads, const std::string &redisUri) :
    producer(producerProps),
    rpcServer(rpcPort),
    redisServer(redisUri)
{
    rpcServer.bind("addGraph", [this](TID id){addGraph(id);});
    rpcServer.bind("addNode", [this](const Node &node){addNode(node);});
    rpcServer.bind("addGraphUnique", [this]{return addGraphUnique();});
    rpcServer.suppress_exceptions(true);

    kafka::clients::AdminClient adminClient(producerProps);
    auto topics = adminClient.listTopics();
    if(topics.error)
        PLOGE << "Could not list topics. Assuming required topics are already created and continuing";
    else
    {
        if(!topics.topics.contains("nodes"))
            adminClient.createTopics({"nodes"}, 100, 1);
    }

    rpcServer.async_run(rpcThreads);
}

void Manager::addGraph(TID id)
{
    PLOGV << "addGraph called with id = " << id;
    std::lock_guard lock(addGraphMutex);
    redisServer.rpush("graphs", std::to_string(id));
}

TID Manager::addGraphUnique()
{
    PLOGV << "addGraphUnique called";
    std::lock_guard lock(addGraphMutex);

    std::vector<std::string> graphs;
    TID id = std::stoi(graphs.back()) + 1;
    redisServer.lrange("graphs", -2, -1, std::back_inserter(graphs));
    redisServer.rpush("graphs", std::to_string(id));

    PLOGV << "addGraphUnique returned id = " << id;

    return id;
}

void Manager::addNode(const Node &node)
{
    PLOGV << "addNode called";
    clmdep_msgpack::sbuffer encodedNode;
    clmdep_msgpack::pack(encodedNode, node);

    kafka::Value value(encodedNode.data(), encodedNode.size());
    kafka::clients::producer::ProducerRecord record("nodes", kafka::NullKey, value);

    try {
        auto md = producer.syncSend(record);
        PLOGV << "Node with id = " << node.id << " has been sent to kafka: " << md.toString();
    }
    catch(kafka::KafkaException &e)
    {
        PLOGW << "Could not send Node to kafka: " << e.what();
        rpc::this_handler().respond_error(e.what());
    }
}

void Manager::setInputValue(TID graphId, TID nodeId, const TData &data)
{
    // Push data to redis
    std::string redisKey = std::to_string(graphId) + "_" + std::to_string(nodeId);
    std::string redisValue(data.ptr, data.size);
    redisServer.set(redisKey, redisValue);

    // Notify kafka
    kafka::Value kafkaValue(redisKey.data(), redisKey.size() * sizeof(char));
    kafka::clients::producer::ProducerRecord record("data", kafka::NullKey, kafkaValue);
    try {
        auto md = producer.syncSend(record);
        PLOGV << "Input for node with id = " << nodeId << " has been set: " << md.toString();
    }
    catch(kafka::KafkaException &e) {
        PLOGW << "Could not send data info to kafka: " << e.what();
        redisServer.del(redisKey);
        rpc::this_handler().respond_error(e.what());
    }
}
