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
    rpcServer.bind("setInputValue", [this](TID graphId, TID nodeId, const std::string &data, MetaData md){setInputValue(graphId, nodeId, data, md);});
    rpcServer.bind("addGraphUnique", [this]{return addGraphUnique();});
    rpcServer.bind("setNodeComputed", [this](TID graphId, TID nodeId){setNodeComputed(graphId, nodeId);});
    rpcServer.bind("getData", [this](TID graphId, TID nodeId){return getData(graphId, nodeId);});
    rpcServer.bind("getMetaData", [this](TID graphId, TID nodeId){return getMetaData(graphId, nodeId);});
    rpcServer.bind("isComputed", [this](TID graphId, TID nodeId){return isComputed(graphId, nodeId);});
    rpcServer.suppress_exceptions(true);

    kafka::clients::AdminClient adminClient(producerProps);
    auto topics = adminClient.listTopics();
    if(topics.error)
        PLOGW << "Could not list topics. Assuming required topics are already created and continuing";
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
    std::lock_guard lock(graphsMutex);
    if(graphs.contains(id))
    {
        PLOGW << "Graph with id = " << id << " already exists";
        throw GraphAlreadyExists();
    }
    else
    {
        maxGraphId = std::max(maxGraphId, id);
        graphs.insert({id, Graph(id)});
    }
}

TID Manager::addGraphUnique()
{
    PLOGV << "addGraphUnique called";
    std::lock_guard lock(graphsMutex);
    ++maxGraphId;
    graphs.insert({maxGraphId, Graph(maxGraphId)});
    PLOGV << "addGraphUnique returned id = " << maxGraphId;
    return maxGraphId;
}

void Manager::addNode(const Node &node)
{
    PLOGV << "addNode called";
    std::unique_lock ulock(graphsMutex);
    auto &graph = graphs.at(node.graphId);
    graph.addNode(node);
    ulock.unlock();
    std::shared_lock slock(graphsMutex);
    if(graph.isReachable(node.id))
        sendNode(node);
}

void Manager::setInputValue(TID graphId, TID nodeId, const std::string &data, MetaData md)
{
    PLOGV << "setInputValue called";
    std::shared_lock slock(graphsMutex);
    auto graphIt = graphs.find(graphId);
    if(graphIt == graphs.end())
        throw GraphDoesNotExist(graphId);

    // Push data to redis
    slock.unlock();
    std::string redisKey = std::to_string(graphId) + "_" + std::to_string(nodeId);
    redisServer.set(redisKey, std::string(data.c_str(), data.size()));

    msgpack::sbuffer encodedMeta;
    msgpack::pack(encodedMeta, md);
    redisServer.set(redisKey + "_meta", std::string(encodedMeta.data(), encodedMeta.size()));

    std::unique_lock ulock(graphsMutex);
    auto &graph = graphIt->second;
    graph.setReady(nodeId);

    // Notify kafka if some node is reachable
    for(auto it = graph.begin(); it != graph.end(); it = graph.nextReachable(it))
    {
        sendNode(graph.at(*it));
    }
}

void Manager::setNodeComputed(TID graphId, TID nodeId)
{
    PLOGV << "setNodeComputed called";
    std::shared_lock slock(graphsMutex);
    auto graphIt = graphs.find(graphId);
    if(graphIt == graphs.end())
        throw GraphDoesNotExist(graphId);

    slock.unlock();
    std::string redisKey = std::to_string(graphId) + "_" + std::to_string(nodeId);
    if(redisServer.exists(redisKey) == 0 /* doesn't exist */)
        throw std::runtime_error("Data is not set");

    std::unique_lock ulock(graphsMutex);
    auto &graph = graphIt->second;
    graph.setReady(nodeId);

    // Notify kafka if some node is reachable
    for(auto it = graph.begin(); it != graph.end(); it = graph.nextReachable(it))
    {
        sendNode(graph.at(*it));
    }
}

void Manager::sendNode(const Node &node)
{
    PLOGV << "sendNode called";
    msgpack::sbuffer encodedNode;
    msgpack::pack(encodedNode, node);

    kafka::Value value(encodedNode.data(), encodedNode.size());
    kafka::clients::producer::ProducerRecord record("nodes", kafka::NullKey, value);

    try {
        auto md = producer.syncSend(record);
        PLOGV << "Node with id = " << node.id << " has been sent to kafka: " << md.toString();
    }
    catch(kafka::KafkaException &e)
    {
        PLOGW << "Could not send Node to kafka: " << e.what();
        std::rethrow_exception(std::current_exception());
    }
}

std::string Manager::getData(TID graphId, TID nodeId)
{
    PLOGV << "getData called";
    std::string redisKey = std::to_string(graphId) + "_" + std::to_string(nodeId);
    if(redisServer.exists(redisKey) == 0 /* doesn't exist */)
        throw DataIsNotSet();

    return redisServer.get(redisKey).value();
}

MetaData Manager::getMetaData(TID graphId, TID nodeId)
{
    PLOGV << "getMetaData called";
    std::string redisKey = std::to_string(graphId) + "_" + std::to_string(nodeId) + "_meta";
    if(redisServer.exists(redisKey) == 0 /* doesn't exist */)
        throw DataIsNotSet();

    auto encodedMeta = redisServer.get(redisKey);
    return msgpack::unpack(encodedMeta->c_str(), encodedMeta->size()).as<MetaData>();
}

bool Manager::isComputed(TID graphId, TID nodeId)
{
    std::shared_lock slock(graphsMutex);
    return graphs.at(graphId).isReady(nodeId);
}
