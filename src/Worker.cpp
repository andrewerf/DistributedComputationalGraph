#include "Worker.h"
#include "Operations.h"
#include "common.h"

#include <plog/Log.h>


Worker::Worker(const kafka::Properties &consumerProps, const std::string &managerRpcHost, const std::string &redisUri):
    consumer(consumerProps),
    rpcClient(extractHostname(managerRpcHost), extractRpcPort(managerRpcHost)),
    redisServer(redisUri)
{
    consumer.subscribe({"nodes"});
}

void Worker::listen()
{
    PLOGI << "Worker is listening";
    while(true)
    {
        auto records = consumer.poll(std::chrono::milliseconds(100));
        PLOGV_IF(!records.empty()) << "Received " << records.size() << " records";

        for(const auto &record : records)
        {
            Node node;
            try {
                node = msgpack::unpack(static_cast<const char*>(record.value().data()), record.value().size()).as<Node>();
            }
            catch(std::exception &e)
            {
                PLOGW << "Could not parse message: " << e.what();
            }

            processNode(node);
        }
    }
}

void Worker::processNode(const Node &node)
{
    std::vector<TensorWithMeta> encodedTensors;
    for(TID inputId : node.inputs)
    {
        TensorWithMeta tensorWithMeta;

        std::string redisKey = std::to_string(node.graphId) + '_' + std::to_string(inputId);
        tensorWithMeta.first = redisServer.get(redisKey).value();

        auto encodedMeta = redisServer.get(redisKey + "_meta");
        tensorWithMeta.second = msgpack::unpack(encodedMeta->c_str(), encodedMeta->size()).as<MetaData>();

        encodedTensors.emplace_back(std::move(tensorWithMeta));
    }

    SqrOp op;
    auto result = op.call(std::move(encodedTensors));

    std::string redisKey = std::to_string(node.graphId) + '_' + std::to_string(node.id);
    redisServer.set(redisKey, result.first);

    msgpack::sbuffer encodedMeta;
    msgpack::pack(encodedMeta, result.second);
    redisServer.set(redisKey + "_meta", std::string(encodedMeta.data(), encodedMeta.size()));

    rpcClient.call("setNodeComputed", node.graphId, node.id);

    PLOGV << "Processed node with id = " << node.id;
}
