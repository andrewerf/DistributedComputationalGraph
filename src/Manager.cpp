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
    rpcServer.bind("setNodeComputed", [this](TID graphId, TID nodeId){setNodeComputed(graphId, nodeId);});
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
    std::lock_guard lock(graphsMutex);
    auto &graph = graphs.at(node.graphId);
    graph.addNode(node);
    if(graph.isReachable(node.id))
        sendNode(node);
}

void Manager::setInputValue(TID graphId, TID nodeId, const msgpack::object_bin &data)
{
    std::unique_lock lock(graphsMutex);
    auto graphIt = graphs.find(graphId);
    if(graphIt == graphs.end())
        throw GraphDoesNotExist(graphId);
    lock.unlock();

    // Push data to redis
    std::string redisKey = std::to_string(graphId) + "_" + std::to_string(nodeId);
    std::string redisValue(data.ptr, data.size);
    redisServer.set(redisKey, redisValue);

    lock.lock();
    auto &graph = graphIt->second;
    graph.setReady(nodeId);

    // Notify kafka if some node is reachable
    for(const auto &reachableId : graph)
    {
        sendNode(graph.at(reachableId));
    }
}

void Manager::setNodeComputed(TID graphId, TID nodeId)
{
    std::unique_lock lock(graphsMutex);
    auto graphIt = graphs.find(graphId);
    if(graphIt == graphs.end())
        throw GraphDoesNotExist(graphId);
    lock.unlock();

    std::string redisKey = std::to_string(graphId) + "_" + std::to_string(nodeId);
    if(redisServer.exists(redisKey) == 0 /* doesn't exist */)
        throw std::runtime_error("Data is not set");

    lock.lock();
    auto &graph = graphIt->second;
    graph.setReady(nodeId);

    // Notify kafka if some node is reachable
    for(const auto &reachableId : graph)
    {
        sendNode(graph.at(reachableId));
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
